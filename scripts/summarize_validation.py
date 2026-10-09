#!/usr/bin/env python3
"""Merge validate_against_cppcheck.py JSON reports into one Markdown table.

Usage: summarize_validation.py out.md name=report.json [name=report.json ...]
Counts are summed across corpora. Precision/recall are measured against
cppcheck's misra.py (an imperfect oracle) at exact file:line positions.
"""
import json
import sys


def main():
    out = sys.argv[1]
    reports = {}
    for item in sys.argv[2:]:
        name, path = item.split("=", 1)
        with open(path) as handle:
            reports[name] = json.load(handle)["rules"]
    rules = sorted({r for rep in reports.values() for r in rep},
                   key=lambda r: [int(p) for p in r.split(".")])
    lines = ["| Rule | misra-checker | cppcheck | both | agree/ours | agree/cppcheck |",
             "|---|---:|---:|---:|---:|---:|"]
    for rule in rules:
        ours = sum(rep.get(rule, {}).get("ours", 0) for rep in reports.values())
        theirs = sum(rep.get(rule, {}).get("cppcheck", 0) for rep in reports.values())
        both = sum(rep.get(rule, {}).get("both", 0) for rep in reports.values())
        p = f"{both / ours:.2f}" if ours else "-"
        r = f"{both / theirs:.2f}" if theirs else "-"
        lines.append(f"| {rule} | {ours} | {theirs} | {both} | {p} | {r} |")
    with open(out, "w") as handle:
        handle.write("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
