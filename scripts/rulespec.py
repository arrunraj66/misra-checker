#!/usr/bin/env python3
"""Per-rule specifications: annotated code cases that define what each detector must and must not report.

A spec file (tests/rulespec/<rule>.spec, for example 8.4.spec) looks like this:

    summary: what the detector reports, in the project's own words
    limits: what it does not detect or approximates (shown on the rule card)

    --- ok: compliant code that must not be reported
    int f(void);
    int f(void) { return 1; }

    --- bad: a violation that must be reported on the marked line
    int g(void) { return 1; }   /*@V*/

    --- gap: a violation the detector misses today (a documented false negative)
    ...code with /*@V*/ markers...

    --- fp: compliant code the detector reports today (a documented false positive)
    ...code, with /*@N*/ markers on the lines that are wrongly reported (or none = anywhere)...

Markers (end of a line):  /*@V*/ a violation of THIS rule is expected on the line,
                          /*@N*/ no report of this rule is allowed on the line.
Kinds:
  ok   : the rule must not be reported anywhere in the case.
  bad  : every /*@V*/ line must be reported (any report, when there are no markers).
  gap  : documents a miss. Holds while at least one /*@V*/ line is NOT reported.
  fp   : documents a false alarm. Holds while the rule is still reported on a /*@N*/ line
         (or anywhere, without markers).
Options after the kind: std=c90|c99|c11, e.g. "--- bad std=c90: description".
Multi-file cases start each file with a line "=== name.c" (whole-program rules need that).

`ok` and `bad` failing means a defect (a regression if it passed before). `gap` and `fp` that no
longer hold mean the problem was fixed: promote the case to `bad` / `ok` (the --check run fails
until you do, so the documented limits stay true).

    rulespec.py [--rule 8.4]... [--check] [--json OUT] [-v]
"""
from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SPEC_DIR = ROOT / "tests" / "rulespec"
FINDING = re.compile(r"^(?P<file>.+?):(?P<line>\d+):(?P<col>\d+): (?P<cat>\w+): MISRA C:2012 Rule (?P<rule>\S+) \[")
HEAD = re.compile(r"^---\s+(?P<kind>ok|bad|gap|fp)(?P<opts>(?:\s+\w+=\S+)*)\s*:\s*(?P<desc>.*)$")
KINDS = ("ok", "bad", "gap", "fp")


class Case:
    def __init__(self, kind: str, desc: str, opts: dict, files: dict[str, str]) -> None:
        self.kind, self.desc, self.opts, self.files = kind, desc, opts, files

    def marks(self, marker: str) -> list[tuple[str, int]]:
        out = []
        for name, text in self.files.items():
            for n, line in enumerate(text.split("\n"), 1):
                if f"/*@{marker}*/" in line:
                    out.append((name, n))
        return out


def parse_spec(path: Path, text: str | None = None, rule: str | None = None) -> dict:
    meta: dict[str, str] = {}
    cases: list[Case] = []
    current = None
    file_name = None
    buffer: list[str] = []

    def flush():
        nonlocal current, file_name, buffer
        if current is not None:
            current["files"][file_name or "main.c"] = "\n".join(buffer).strip("\n") + "\n"
        buffer, file_name = [], None

    for raw in (text if text is not None else path.read_text()).split("\n"):
        head = HEAD.match(raw)
        if head:
            flush()
            if current is not None:
                cases.append(Case(current["kind"], current["desc"], current["opts"], current["files"]))
            opts = dict(o.split("=", 1) for o in head["opts"].split())
            current = {"kind": head["kind"], "desc": head["desc"].strip(), "opts": opts, "files": {}}
            continue
        if raw.startswith("---"):
            raise ValueError(f"{path.name}: bad case header: {raw!r}")
        if current is None:
            m = re.match(r"^(summary|limits|notes):\s*(.*)$", raw)
            if m:
                meta[m[1]] = m[2].strip()
            elif raw.strip() and not raw.startswith("#"):
                if meta and raw.startswith((" ", "\t")):
                    last = list(meta)[-1]
                    meta[last] += " " + raw.strip()
                else:
                    raise ValueError(f"{path.name}: unexpected line before the first case: {raw!r}")
            continue
        m = re.match(r"^===\s*(\S+)\s*$", raw)
        if m:
            if buffer or file_name:
                current["files"][file_name or "main.c"] = "\n".join(buffer).strip("\n") + "\n"
            file_name, buffer = m[1], []
            continue
        buffer.append(raw)
    flush()
    if current is not None:
        cases.append(Case(current["kind"], current["desc"], current["opts"], current["files"]))
    return {"rule": rule or path.stem, "meta": meta, "cases": cases}


def run_case(checker: str, clang: str, rule: str, case: Case) -> dict:
    """Result: {"status": pass|fail|open|fixed, "detail": str, "reported": [lines]}."""
    std = case.opts.get("std", "c99")
    with tempfile.TemporaryDirectory(prefix="rulespec-") as tmp:
        root = Path(tmp).resolve()
        for name, text in case.files.items():
            (root / name).write_text(text)
        units = sorted(n for n in case.files if n.endswith(".c"))
        db = [{"directory": str(root), "file": str(root / u), "arguments": [clang, f"-std={std}", f"-I{root}", "-c", str(root / u)]} for u in units]
        (root / "compile_commands.json").write_text(json.dumps(db))
        proc = subprocess.run([checker, "analyze", "--compile-commands", str(root / "compile_commands.json")],
                              capture_output=True, text=True, timeout=120)
        prefix = str(root) + "/"
        reported = set()
        for line in proc.stdout.splitlines():
            m = FINDING.match(line)
            if m and m["rule"] == rule and m["cat"] != "deviated":
                reported.add((m["file"].replace(prefix, ""), int(m["line"])))
    want, forbid = case.marks("V"), case.marks("N")
    missed = [w for w in want if w not in reported]
    wrongly = [f for f in forbid if f in reported] if forbid else sorted(reported)
    kind = case.kind
    if kind == "ok":
        ok = not reported
        return {"status": "pass" if ok else "fail", "detail": "" if ok else f"reported at {sorted(reported)}", "reported": sorted(reported)}
    if kind == "bad":
        ok = (not missed) if want else bool(reported)
        return {"status": "pass" if ok else "fail", "detail": "" if ok else (f"missed {missed}" if want else "nothing reported"), "reported": sorted(reported)}
    if kind == "gap":
        holds = bool(missed) if want else not reported
        return {"status": "open" if holds else "fixed", "detail": "" if holds else "now reported: promote to bad", "reported": sorted(reported)}
    holds = bool(wrongly)                                     # fp
    return {"status": "open" if holds else "fixed", "detail": "" if holds else "no longer reported: promote to ok", "reported": sorted(reported)}


def load_specs(rules: list[str] | None) -> list[dict]:
    """One rule per `<rule>.spec`, or many in a `*.specs` file split by lines `##### rule X.Y`."""
    specs = []
    for path in sorted(SPEC_DIR.glob("*.spec")):
        specs.append(parse_spec(path))
    for path in sorted(SPEC_DIR.glob("*.specs")):
        parts = re.split(r"^#####\s*rule\s+(\d+\.\d+)\s*$", path.read_text(), flags=re.M)
        for i in range(1, len(parts), 2):
            specs.append(parse_spec(path, parts[i + 1], parts[i]))
    seen = set()
    for spec in specs:
        if spec["rule"] in seen:
            raise ValueError(f"rule {spec['rule']} is specified twice")
        seen.add(spec["rule"])
    specs = [s for s in specs if not rules or s["rule"] in rules]
    return sorted(specs, key=lambda s: tuple(int(x) for x in s["rule"].split(".")))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--rule", action="append", help="only this rule (repeatable)")
    ap.add_argument("--checker", default=str(ROOT / "build" / "misra-checker"))
    ap.add_argument("--clang", default=shutil.which("clang-14") or shutil.which("clang") or "clang")
    ap.add_argument("--check", action="store_true", help="exit 1 on a failing ok/bad case or a gap/fp that no longer holds")
    ap.add_argument("--json", type=Path, help="write all results as JSON")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()

    specs = load_specs(args.rule)
    jobs = [(s["rule"], i, c) for s in specs for i, c in enumerate(s["cases"])]
    with ThreadPoolExecutor(max_workers=8) as pool:
        results = list(pool.map(lambda j: run_case(args.checker, args.clang, j[0], j[2]), jobs))
    counts = {"pass": 0, "fail": 0, "open": 0, "fixed": 0}
    bad = []
    out = {}
    for (rule, i, case), result in zip(jobs, results):
        counts[result["status"]] += 1
        out.setdefault(rule, []).append({"kind": case.kind, "desc": case.desc, **result})
        if result["status"] in ("fail", "fixed"):
            bad.append((rule, case, result))
        if args.verbose and result["status"] != "pass":
            print(f"  {rule:<6} {case.kind:<4} {result['status']:<6} {case.desc[:70]}  {result['detail']}")
    for rule, case, result in bad:
        print(f"{result['status'].upper():<6} rule {rule} [{case.kind}] {case.desc}: {result['detail']}")
    rules_done = len(specs)
    print(f"\n{rules_done} rule spec(s), {len(jobs)} case(s): {counts['pass']} pass, {counts['fail']} fail, "
          f"{counts['open']} documented limit(s), {counts['fixed']} to promote")
    if args.json:
        args.json.write_text(json.dumps(out, indent=1))
    return 1 if (args.check and (counts["fail"] or counts["fixed"])) else 0


if __name__ == "__main__":
    sys.exit(main())
