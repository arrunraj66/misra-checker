#!/usr/bin/env python3
"""Quality gate for CI: fail the build on NEW violations, not on legacy ones.

    misra_ci.py --compile-commands build/compile_commands.json --baseline misra-baseline.json
    misra_ci.py --compile-commands DB --baseline misra-baseline.json --update-baseline   # accept the current state

Runs misra-checker, gives every finding a stable fingerprint (rule, file, line text; see baseline.py),
compares with the committed baseline and exits 1 when a finding at or above --min-category is new.
Findings covered by a deviation (--deviations) never count. Without --baseline every finding counts.

With --publish URL --project NAME the run is also sent to the shared history of a misra-checker server
(token from --token or the MISRA_HISTORY_TOKEN environment variable), so trends are visible to the team.

Exit status: 0 gate passed, 1 gate failed, 2 the checker could not run.
"""
import argparse
import json
import os
import re
import subprocess
import sys
import urllib.error
import urllib.request
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import baseline as bl  # noqa: E402

LINE = re.compile(r"^(?P<file>.+?):(?P<line>\d+):(?P<col>\d+): (?P<cat>\w+): MISRA C:2012 Rule (?P<rule>\S+) \[(?P<key>[^\]]+)\]")


def run_checker(checker: str, database: str, extra: list[str]) -> tuple[list[dict], str, int]:
    proc = subprocess.run([checker, "analyze", "--compile-commands", database, *extra], capture_output=True, text=True)
    findings, seen = [], set()
    run_checker.deviated = 0
    for line in proc.stdout.splitlines():
        m = LINE.match(line)
        if m and m["cat"] == "deviated":
            run_checker.deviated += 1
        if not m or m["cat"] == "deviated":
            continue
        item = {"file": m["file"], "line": int(m["line"]), "col": int(m["col"]), "category": m["cat"], "rule": m["rule"],
                "message": re.sub(r"^misra-c2012-\d+\.\d+-", "", m["key"]).replace("-", " ")}
        key = (item["file"], item["line"], item["col"], item["rule"], m["key"])
        if key not in seen:
            seen.add(key)
            findings.append(item)
    return findings, proc.stderr, proc.returncode


def publish(url: str, token: str, payload: dict) -> str:
    request = urllib.request.Request(url.rstrip("/") + "/api/runs", data=json.dumps(payload).encode(), method="POST",
                                     headers={"Content-Type": "application/json", "Authorization": f"Bearer {token}"})
    try:
        with urllib.request.urlopen(request, timeout=120) as response:
            run = json.load(response)["run"]
    except urllib.error.HTTPError as exc:
        raise RuntimeError(f"server answered {exc.code}: {exc.read().decode(errors='replace')[:200]}") from None
    except (urllib.error.URLError, OSError) as exc:
        raise RuntimeError(f"cannot reach the server: {exc}") from None
    return f"run {run['id']} stored for {run['project']}/{run['branch']}: {run['total']} findings, {run['new_count']} new, {run['fixed_count']} fixed since the previous run"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--compile-commands", required=True)
    ap.add_argument("--checker", default=str(Path(__file__).resolve().parent.parent / "build" / "misra-checker"))
    ap.add_argument("--baseline", type=Path, help="baseline JSON (committed to the repository)")
    ap.add_argument("--update-baseline", action="store_true", help="write the current findings as the new baseline")
    ap.add_argument("--min-category", choices=list(bl.CATEGORY_RANK), default="required",
                    help="lowest category that can fail the gate (default: required)")
    ap.add_argument("--fail-on", choices=["new", "any", "none"], default=None,
                    help="new = only findings absent from the baseline (default when a baseline exists); any = every finding")
    ap.add_argument("--source-root", type=Path, default=Path.cwd(), help="paths in the baseline are relative to this")
    ap.add_argument("--deviations", help="deviation file passed to misra-checker")
    ap.add_argument("--sarif-out", help="also write SARIF here")
    ap.add_argument("--json-out", type=Path, help="write the comparison as JSON")
    ap.add_argument("--publish", metavar="URL", help="send the run to a misra-checker server's shared history")
    ap.add_argument("--project", help="project name in the shared history (needed with --publish)")
    ap.add_argument("--branch", default=os.environ.get("GITHUB_REF_NAME", "main"))
    ap.add_argument("--commit", default=os.environ.get("GITHUB_SHA", "")[:40])
    ap.add_argument("--token", default=os.environ.get("MISRA_HISTORY_TOKEN", ""), help="API token (or MISRA_HISTORY_TOKEN)")
    ap.add_argument("--require-publish", action="store_true", help="fail (exit 2) when publishing does not work")
    args = ap.parse_args()

    extra = []
    if args.deviations:
        extra += ["--deviations", args.deviations]
    if args.sarif_out:
        extra += ["--sarif-out", args.sarif_out, "--source-root", str(args.source_root)]
    findings, stderr, code = run_checker(args.checker, args.compile_commands, extra)
    if code not in (0, 1) and not findings and "results may be incomplete" not in stderr:
        print(f"misra-checker could not run (exit {code}): {stderr.strip()[:500]}", file=sys.stderr)
        return 2
    if "results may be incomplete" in stderr:
        print("warning: the code has compile errors; results may be incomplete", file=sys.stderr)

    root = args.source_root.resolve()
    cache: dict[str, list[str]] = {}

    def line_text(file: str, line: int) -> str:
        if file not in cache:
            try:
                cache[file] = Path(file).read_text(errors="replace").split("\n")
            except OSError:
                cache[file] = []
        lines = cache[file]
        return lines[line - 1] if 0 < line <= len(lines) else ""

    for f in findings:   # remember the absolute path for reading, store a portable relative one
        absolute = f["file"]
        f["_abs"] = absolute
        try:
            f["file"] = os.path.relpath(absolute, root) if Path(absolute).is_absolute() else absolute
        except ValueError:
            pass
    absolute_of = {f["file"]: f["_abs"] for f in findings}
    bl.fingerprint_all(findings, lambda file, line: line_text(absolute_of.get(file, file), line))

    previous = []
    if args.baseline and args.baseline.is_file():
        data = json.loads(args.baseline.read_text())
        snaps = data.get("snapshots")
        previous = (snaps[-1] if snaps else data).get("findings", [])
    fail_on = args.fail_on or ("new" if args.baseline else "any")
    summary = bl.compare(findings, previous) if args.baseline else {
        "new": len(findings), "unchanged": 0, "fixed": 0, "fixed_entries": [],
        "new_by_category": {c: sum(1 for f in findings if f["category"] == c) for c in bl.CATEGORY_RANK}}

    if args.update_baseline:
        if not args.baseline:
            print("--update-baseline needs --baseline FILE", file=sys.stderr)
            return 2
        args.baseline.write_text(json.dumps(bl.snapshot(findings), indent=1) + "\n")
        print(f"baseline written: {len(findings)} finding(s) -> {args.baseline}")
        return 0

    print(f"{len(findings)} finding(s): {summary['new']} new, {summary['unchanged']} unchanged, {summary['fixed']} fixed since the baseline")
    for f in sorted(findings, key=lambda f: (f["file"], f["line"])):
        if f.get("status", "new") == "new" and bl.CATEGORY_RANK.get(f["category"], 1) >= bl.CATEGORY_RANK[args.min_category]:
            print(f"  NEW {f['category']:<9} {f['file']}:{f['line']}: Rule {f['rule']} ({f['message']})")
    blocking = (bl.worst_new(summary, args.min_category) if fail_on == "new" else
                sum(1 for f in findings if bl.CATEGORY_RANK.get(f["category"], 1) >= bl.CATEGORY_RANK[args.min_category])
                if fail_on == "any" else 0)
    if args.publish:
        if not args.project or not args.token:
            print("--publish needs --project and a token (--token or MISRA_HISTORY_TOKEN)", file=sys.stderr)
            return 2
        version = subprocess.run([args.checker, "--version"], capture_output=True, text=True).stdout.strip()
        try:
            print(publish(args.publish, args.token, {
                "project": args.project, "branch": args.branch, "commit": args.commit, "tool_version": version,
                "deviated": getattr(run_checker, "deviated", 0),
                "findings": [{k: v for k, v in f.items() if k in ("fp", "rule", "file", "line", "category", "message", "text")} for f in findings]}))
        except RuntimeError as exc:
            print(f"warning: could not publish the run: {exc}", file=sys.stderr)
            if args.require_publish:
                return 2
    if args.json_out:
        args.json_out.write_text(json.dumps({"summary": {k: v for k, v in summary.items() if k != "fixed_entries"},
                                             "fixed": summary["fixed_entries"], "blocking": blocking,
                                             "findings": [{k: v for k, v in f.items() if k != "_abs"} for f in findings]}, indent=1))
    if blocking:
        print(f"GATE FAILED: {blocking} {'new ' if fail_on == 'new' else ''}finding(s) at or above '{args.min_category}'")
        return 1
    print("gate passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
