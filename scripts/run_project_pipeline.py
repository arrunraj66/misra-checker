#!/usr/bin/env python3
"""Zip in, report out: find MISRA findings, ask an AI provider to fix them, gate the
fixes, and write a report.

    run_project_pipeline.py PROJECT.zip --out OUT_DIR --checker build/misra-checker \
        --provider-cmd "python3 scripts/ai_providers/ollama_provider.py" --license me.lic

Steps: unzip safely -> compile database -> analyze -> `misra-checker convert` once
per file that has findings -> apply the accepted proposals to a COPY of the project
-> analyze the copy -> write report.html / report.json / findings.csv and
fixed_project.zip. The original sources are never modified. The converter's gates
prove "compiles, targeted findings gone, nothing worse, --verify-cmd passes", not
that behaviour is unchanged: read every patch.
"""
import argparse
import csv
import fnmatch
import html
import json
import re
import shutil
import subprocess
import sys
import zipfile
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
FINDING = re.compile(
    r"^(?P<file>.+?):(?P<line>\d+):(?P<col>\d+): (?P<sev>\w+): MISRA C:2012 Rule "
    r"(?P<rule>[\d.]+) \[(?P<key>[^\]]+)\]")


# Rules each converter fix class can address (mirrors src/converter.cpp; keep in sync).
CLASS_RULES = {
    "literal-hygiene": {"7.1", "7.2", "7.3"},
    "goto-elimination": {"15.1", "15.2", "15.3"},
    "structured-control-flow": {"14.2", "15.5", "15.6", "15.7", "16.3", "16.4", "16.5", "16.6"},
    "boolean-and-side-effects": {"12.3", "13.4", "13.5", "13.6", "14.4"},
    "call-and-parameter-hygiene": {"2.7", "17.7", "17.8"},
    "declaration-hygiene": {"8.2", "8.8", "8.10", "8.11", "8.14"},
    "pointer-cast-hygiene": {"7.4", "11.5", "11.8", "11.9"},
}
FIXABLE = set().union(*CLASS_RULES.values())


def safe_extract(archive: Path, dest: Path) -> None:
    root = dest.resolve()
    with zipfile.ZipFile(archive) as zf:
        for member in zf.infolist():
            target = (dest / member.filename).resolve()
            if root != target and root not in target.parents:
                raise SystemExit(f"refusing zip entry outside the project: {member.filename}")
        zf.extractall(dest)


def run_analyze(checker: str, database: Path):
    done = subprocess.run([checker, "analyze", "--compile-commands", str(database)],
                          capture_output=True, text=True)
    findings, notices = [], []
    for line in (done.stdout + done.stderr).splitlines():
        match = FINDING.match(line)
        if match:
            findings.append({"file": match["file"], "line": int(match["line"]),
                             "col": int(match["col"]), "severity": match["sev"],
                             "rule": match["rule"], "key": match["key"]})
        elif line.strip():
            notices.append(line)
    return done.returncode, findings, notices


def make_db(project: Path, std: str, defines) -> Path:
    cmd = [sys.executable, str(HERE / "make_compile_db.py"), str(project), "--std", std]
    for define in defines:
        cmd += ["-D", define]
    subprocess.run(cmd, check=True)
    return project / "compile_commands.json"


def convert_file(args, database: Path, source: str, out_dir: Path, fix_class: str) -> dict:
    out_dir.mkdir(parents=True, exist_ok=True)
    cmd = [args.checker, "convert", "--compile-commands", str(database),
           "--file", source, "--provider-cmd", args.provider_cmd,
           "--verify-cmd", args.verify_cmd, "--output-dir", str(out_dir),
           "--license", str(args.license), "--fix-class", fix_class]
    try:
        done = subprocess.run(cmd, capture_output=True, text=True, timeout=args.timeout)
        message = (done.stdout + done.stderr).strip().splitlines()
        message = message[0] if message else ""
    except subprocess.TimeoutExpired:
        message = f"timed out after {args.timeout}s"
    audit = out_dir / "audit.jsonl"
    record = {}
    if audit.exists():
        lines = [json.loads(x) for x in audit.read_text().splitlines() if x.strip()]
        record = lines[-1] if lines else {}
    proposed = sorted(out_dir.glob("*.proposed"))
    patches = sorted(out_dir.glob("*.patch"))
    return {"file": source, "outcome": record.get("outcome", "error"),
            "detail": record.get("detail", message), "fix_class": record.get("fix_class", ""),
            "before": record.get("findings_before"), "after": record.get("findings_after"),
            "proposed": str(proposed[0]) if proposed else "",
            "patch": patches[0].read_text() if patches else ""}


def write_report(out: Path, summary: dict, results, findings_before, findings_after, notices):
    (out / "report.json").write_text(json.dumps(
        {"summary": summary, "files": results, "findings_before": findings_before,
         "findings_after": findings_after, "notices": notices}, indent=1))
    with open(out / "findings.csv", "w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["stage", "file", "line", "col", "severity", "rule", "key"])
        for stage, rows in (("before", findings_before), ("after", findings_after)):
            for f in rows:
                writer.writerow([stage, f["file"], f["line"], f["col"], f["severity"],
                                 f["rule"], f["key"]])
    esc = html.escape
    by_rule_b = Counter(f["rule"] for f in findings_before)
    by_rule_a = Counter(f["rule"] for f in findings_after)
    rule_rows = "".join(
        f"<tr><td>{esc(r)}</td><td>{by_rule_b[r]}</td><td>{by_rule_a.get(r, 0)}</td></tr>"
        for r in sorted(by_rule_b, key=lambda x: [int(p) for p in x.split(".")]))
    file_rows = "".join(
        f"<tr><td>{esc(r['file'])}</td><td>{esc(r['outcome'])}</td><td>{esc(r['fix_class'])}</td>"
        f"<td>{r['before'] if r['before'] is not None else ''}</td>"
        f"<td>{r['after'] if r['after'] is not None else ''}</td><td>{esc(r['detail'])}</td></tr>"
        for r in results)
    patches = "".join(
        f"<h3>{esc(r['file'])}</h3><pre>{esc(r['patch'])}</pre>" for r in results if r["patch"])
    fixable_note = (
        "<p>Findings of rules with <b>no automatic fix class</b> are never changed by the converter "
        "(for example 8.4, 8.7, 2.x, 5.x, 9.x, 10.x, 18.x, 20.x, 21.x). They stay in the fixed project "
        "and need manual work. Rules the converter can attempt: "
        + esc(", ".join(sorted(FIXABLE, key=lambda x: [int(p) for p in x.split(".")]))) + ".</p>")
    note_html = "".join(f"<li>{esc(n)}</li>" for n in notices[:50])
    (out / "report.html").write_text(f"""<!doctype html><meta charset="utf-8">
<title>MISRA conversion report</title>
<style>body{{font:14px system-ui;margin:2em;max-width:1100px}}table{{border-collapse:collapse}}
td,th{{border:1px solid #bbb;padding:4px 8px;text-align:left}}pre{{background:#f4f4f4;padding:8px;overflow:auto}}</style>
<h1>MISRA C:2012 conversion report</h1>
<p><b>This is not a compliance claim.</b> The checker covers only its implemented rules, and the
converter's gates prove "compiles, targeted findings gone, nothing worse", not unchanged behaviour.
Review every patch before using it.</p>
<h2>Summary</h2>
<ul>{"".join(f"<li>{esc(k)}: <b>{esc(str(v))}</b></li>" for k, v in summary.items())}</ul>
<h2>Findings per rule (before / after applying accepted fixes)</h2>
<table><tr><th>Rule</th><th>Before</th><th>After</th></tr>{rule_rows}</table>
{fixable_note}<h2>Files</h2>
<table><tr><th>File</th><th>Outcome</th><th>Fix class</th><th>Before</th><th>After</th><th>Detail</th></tr>{file_rows}</table>
<h2>Notices</h2><ul>{note_html}</ul>
<h2>Patches</h2>{patches}""")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("zip", type=Path)
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--checker", default="build/misra-checker")
    ap.add_argument("--provider-cmd", required=True)
    ap.add_argument("--license", type=Path, required=True)
    ap.add_argument("--verify-cmd", default="true")
    ap.add_argument("--std", default="c99", choices=["c90", "c99", "c11", "c17"])
    ap.add_argument("-D", dest="defines", action="append", default=[])
    ap.add_argument("--exclude", action="append", default=[],
                    help="glob (relative path) of files not to convert, e.g. 'lib/*'")
    ap.add_argument("--max-files", type=int, default=0, help="0 = all")
    ap.add_argument("--timeout", type=int, default=1800, help="seconds per file")
    ap.add_argument("--passes", type=int, default=4,
                    help="repeat find/fix on the fixed copy until nothing changes (max passes)")
    args = ap.parse_args()

    out = args.out.resolve()
    if out.exists():
        shutil.rmtree(out)
    work = out / "project"
    work.mkdir(parents=True)
    safe_extract(args.zip, work)
    project = work
    shutil.rmtree(work / "__MACOSX", ignore_errors=True)
    database = make_db(project, args.std, args.defines)

    code, findings_before, notices = run_analyze(args.checker, database)
    if code not in (0, 1):
        print("analysis failed:\n" + "\n".join(notices[:20]), file=sys.stderr)
    per_file = Counter(f["file"] for f in findings_before)
    todo = [f for f, _ in per_file.most_common()
            if not any(fnmatch.fnmatch(str(Path(f).relative_to(project)), g) for g in args.exclude)]
    if args.max_files:
        todo = todo[:args.max_files]
    print(f"{len(findings_before)} findings in {len(per_file)} files; converting {len(todo)}")

    fixed = out / "fixed_project"
    shutil.copytree(project, fixed, ignore=shutil.ignore_patterns("compile_commands.json"))
    fixed_db = make_db(fixed, args.std, args.defines)
    allowed = {str(fixed / Path(f).relative_to(project)) for f in todo}

    results, passes_used, job = [], 0, 0
    for pass_no in range(1, args.passes + 1):
        _, current, _ = run_analyze(args.checker, fixed_db)
        by_file = {}
        for f in current:
            if f["file"] in allowed:
                by_file.setdefault(f["file"], set()).add(f["rule"])
        work_list = [(src, [c for c, rules in CLASS_RULES.items() if rules & r])
                     for src, r in by_file.items()]
        work_list = [(src, classes) for src, classes in work_list if classes]
        if not work_list:
            break
        passes_used = pass_no
        changed = 0
        print(f"--- pass {pass_no}: {len(work_list)} files with fixable findings", flush=True)
        for source, classes in work_list:
            rel = Path(source).relative_to(fixed)
            for fix_class in classes:
                job += 1
                print(f"[{job}] pass {pass_no} {rel} ({fix_class})", flush=True)
                r = convert_file(args, fixed_db, source, out / "converter" / f"{job:04d}", fix_class)
                r["pass"] = pass_no
                results.append(r)
                print("   ", r["outcome"], "-", r["detail"], flush=True)
                if r["outcome"] == "proposed" and r["proposed"]:
                    shutil.copyfile(r["proposed"], source)
                    changed += 1
                    break          # re-analyze before trying this file's other classes
        if not changed:
            break

    _, findings_after, notices_after = run_analyze(args.checker, fixed_db)
    unfixable = Counter(f["rule"] for f in findings_after if f["rule"] not in FIXABLE)
    summary = {"files with findings": len(per_file), "files attempted": len(todo),
               "passes run": passes_used,
               "fixes accepted by the gates": sum(r["outcome"] == "proposed" for r in results),
               "fixes rejected or failed": sum(r["outcome"] != "proposed" for r in results),
               "findings before": len(findings_before),
               "findings after accepted fixes": len(findings_after),
               "remaining with no automatic fix class": sum(unfixable.values())}
    for rows, base in ((findings_before, project), (findings_after, fixed)):
        for f in rows:
            f["file"] = str(Path(f["file"]).relative_to(base))
    for r in results:
        r["file"] = str(Path(r["file"]).relative_to(fixed))
    write_report(out, summary, results, findings_before, findings_after, notices + notices_after)
    shutil.make_archive(str(out / "fixed_project"), "zip", fixed)
    print(json.dumps(summary, indent=1))
    print(f"report: {out / 'report.html'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
