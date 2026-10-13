#!/usr/bin/env python3
"""Compare misra-checker with an independent oracle: a code base whose authors
publish their MISRA C:2012 deviations.

The oracle used here is the FreeRTOS-Kernel: its maintainers state conformance to
MISRA C:2012 (checked with Coverity) and mark every remaining violation in the
source with `coverity[misra_c_2012_rule_X_Y_violation]` comments plus a
project-wide list of deviated rules. For each rule that is NOT deviated project-wide:

* a finding next to a marker for the same rule      -> documented (agreement)
* a finding with no marker                          -> unexplained (suspected false
                                                       positive until triaged)
* a marker with no finding of that rule nearby      -> missed (suspected false negative)

"Unexplained" is not automatically wrong: the oracle only covers the rules and
configuration its authors analysed. Every group must be triaged by a person; this
script produces the evidence (counts per rule and the exact lines).

    validate_oracle.py --compile-commands DB --source-root DIR \\
        --deviation-config coverity_misra.config --report-dir OUT
"""
import argparse
import csv
import json
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

FINDING = re.compile(r"^(?P<file>.+?):(?P<line>\d+):(?P<col>\d+): (?P<cat>\w+): "
                     r"MISRA C:2012 Rule (?P<rule>\S+) \[(?P<key>[^\]]+)\]")
MARKER = re.compile(r"coverity\[misra_c_2012_rule_(\d+)_(\d+)_violation\]")
COMMENT_ONLY = re.compile(r"^\s*(/\*.*\*/\s*|//.*)?$")
SPAN = 8  # lines after the first code line that a marker is taken to cover


def run_checker(checker, db):
    proc = subprocess.run([checker, "analyze", "--compile-commands", db],
                          capture_output=True, text=True)
    findings = []
    for line in proc.stdout.splitlines():
        m = FINDING.match(line)
        if m and m["cat"] != "deviated":
            findings.append({"file": m["file"], "line": int(m["line"]), "rule": m["rule"], "key": m["key"]})
    return findings, proc


def compiled_lines(db_path):
    """{file: set(line numbers)} that survive preprocessing in at least one TU, using
    the `# N "file"` line markers of `clang -E`. Lines in inactive #if groups are absent."""
    compiled = defaultdict(set)
    for entry in json.loads(Path(db_path).read_text()):
        args = [a for a in entry["arguments"] if a != "-c" and a != entry["file"] and not a.startswith("-o")]
        proc = subprocess.run([*args, "-E", entry["file"]], capture_output=True, text=True)
        current, number = None, 0
        for line in proc.stdout.split("\n"):
            m = re.match(r'^# (\d+) "([^"]*)"', line)
            if m:
                number, current = int(m[1]), m[2]
                continue
            if current is not None and line.strip():
                compiled[current].add(number)
            number += 1
    return compiled


def markers_in(root):
    markers = []
    for path in sorted(root.rglob("*")):
        if path.suffix not in (".c", ".h") or not path.is_file():
            continue
        lines = path.read_text(errors="replace").split("\n")
        for i, text in enumerate(lines):
            m = MARKER.search(text)
            if not m:
                continue
            j = i + 1
            while j < len(lines) and COMMENT_ONLY.match(lines[j]):
                j += 1
            markers.append({"file": str(path), "rule": f"{m[1]}.{m[2]}", "line": j + 1})
    return markers


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--checker", default="build/misra-checker")
    ap.add_argument("--compile-commands", required=True)
    ap.add_argument("--source-root", required=True, type=Path)
    ap.add_argument("--deviation-config", type=Path, help="JSON with a 'deviations' list of {'deviation': 'Rule X.Y'}")
    ap.add_argument("--report-dir", required=True, type=Path)
    ap.add_argument("--name", default="oracle")
    args = ap.parse_args()

    root = args.source_root.resolve()
    project_deviated = set()
    if args.deviation_config:
        for d in json.loads(args.deviation_config.read_text())["deviations"]:
            m = re.fullmatch(r"Rule (\d+\.\d+)", d["deviation"])
            if m:
                project_deviated.add(m[1])

    findings, proc = run_checker(args.checker, args.compile_commands)
    findings = [f for f in findings if f["file"].startswith(str(root))]
    markers = markers_in(root)

    by_rule = defaultdict(lambda: defaultdict(int))
    unexplained = []
    for f in findings:
        rule = f["rule"]
        if rule in project_deviated:
            f["class"] = "project-deviated"
        elif any(m["rule"] == rule and m["file"] == f["file"] and m["line"] <= f["line"] <= m["line"] + SPAN
                 for m in markers):
            f["class"] = "documented"
        else:
            f["class"] = "unexplained"
            unexplained.append(f)
        by_rule[rule][f["class"]] += 1
    missed, inactive = [], []
    compiled = compiled_lines(args.compile_commands)
    line_cache = {}

    def text_of(path, n):
        if path not in line_cache:
            line_cache[path] = Path(path).read_text(errors="replace").split("\n")
        return line_cache[path][n - 1].strip() if 0 < n <= len(line_cache[path]) else ""

    for m in markers:
        hit = any(f["rule"] == m["rule"] and f["file"] == m["file"] and m["line"] <= f["line"] <= m["line"] + SPAN
                  for f in findings)
        by_rule[m["rule"]]["markers"] += 1
        if hit:
            by_rule[m["rule"]]["detected"] += 1
        elif m["line"] not in compiled.get(m["file"], set()):
            by_rule[m["rule"]]["inactive"] += 1   # not compiled in this configuration
            inactive.append(m)
        else:
            by_rule[m["rule"]]["missed"] += 1
            if m["rule"] not in project_deviated:
                missed.append(m)

    out = args.report_dir
    out.mkdir(parents=True, exist_ok=True)
    cache = {}

    def source_line(path, n):
        if path not in cache:
            cache[path] = Path(path).read_text(errors="replace").split("\n")
        return cache[path][n - 1].strip() if 0 < n <= len(cache[path]) else ""

    with open(out / f"{args.name}-unexplained.csv", "w", newline="") as handle:
        w = csv.writer(handle)
        w.writerow(["rule", "file", "line", "check", "source"])
        for f in sorted(unexplained, key=lambda x: (tuple(map(int, x["rule"].split("."))), x["file"], x["line"])):
            w.writerow([f["rule"], str(Path(f["file"]).relative_to(root)), f["line"], f["key"], source_line(f["file"], f["line"])])
    summary = {
        "findings": len(findings), "documented": sum(r["documented"] for r in by_rule.values()),
        "project_deviated": sum(r["project-deviated"] for r in by_rule.values()),
        "unexplained": len(unexplained), "markers": len(markers),
        "markers_detected": sum(r["detected"] for r in by_rule.values()),
        "markers_missed": len(missed), "markers_inactive": len(inactive),
        "rules": {r: dict(v) for r, v in sorted(by_rule.items(), key=lambda kv: tuple(map(int, kv[0].split("."))))},
        "missed": [{"rule": m["rule"], "file": str(Path(m["file"]).relative_to(root)), "line": m["line"],
                    "source": source_line(m["file"], m["line"])} for m in missed],
    }
    (out / f"{args.name}.json").write_text(json.dumps(summary, indent=2))

    print(f"{args.name}: {summary['findings']} findings = {summary['documented']} documented + "
          f"{summary['project_deviated']} in project-deviated rules + {summary['unexplained']} unexplained")
    print(f"{summary['markers']} documented deviations: {summary['markers_detected']} detected, "
          f"{summary['markers_missed']} missed, {summary['markers_inactive']} in code not compiled in this configuration")
    print(f"{'rule':<7}{'findings':>9}{'documented':>11}{'proj-dev':>9}{'unexplained':>12}{'markers':>8}{'missed':>7}{'inactive':>9}")
    for rule, v in summary["rules"].items():
        total = v.get("documented", 0) + v.get("project-deviated", 0) + v.get("unexplained", 0)
        print(f"{rule:<7}{total:>9}{v.get('documented', 0):>11}{v.get('project-deviated', 0):>9}"
              f"{v.get('unexplained', 0):>12}{v.get('markers', 0):>8}{v.get('missed', 0):>7}{v.get('inactive', 0):>9}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
