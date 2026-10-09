#!/usr/bin/env python3
"""Differential validation of misra-checker against cppcheck's MISRA addon.

cppcheck's misra.py is an independent, open implementation of many MISRA
C:2012 rules. It is NOT ground truth: where the two tools disagree either one
can be wrong. The report therefore gives agreement figures and lists sample
disagreements for manual triage, per rule.

Usage:
  validate_against_cppcheck.py --checker build/misra-checker \
      --compile-commands db.json --out report.json [--samples 3]
Needs cppcheck (with its addons directory) on the machine.
"""
import argparse
import collections
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile

ADDON_DIRS = ["/usr/lib/x86_64-linux-gnu/cppcheck/addons", "/usr/share/cppcheck/addons"]
OURS = re.compile(r"^(.*?):(\d+):\d+: \w+: MISRA C:2012 Rule (\d+\.\d+) \[")
THEIRS = re.compile(r"\[([^\]:]+):(\d+)\].*\[misra-c2012-(\d+\.\d+)\]")


def find_misra():
    for directory in ADDON_DIRS:
        path = os.path.join(directory, "misra.py")
        if os.path.exists(path):
            return path
    sys.exit("cppcheck misra.py addon not found")


def run_ours(checker, database):
    result = subprocess.run([checker, "analyze", "--compile-commands", database],
                            capture_output=True, text=True)
    found = collections.defaultdict(set)
    for line in result.stdout.splitlines():
        match = OURS.match(line)
        if match:
            found[match.group(3)].add((os.path.basename(match.group(1)), int(match.group(2))))
    return found, result.stdout.strip().splitlines()[-2:]


def run_theirs(entry, misra, workdir):
    arguments = entry.get("arguments") or shlex.split(entry["command"])
    flags = [a for a in arguments if a.startswith(("-I", "-D"))]
    source = entry["file"]
    base = os.path.join(workdir, os.path.basename(source))
    subprocess.run(["cppcheck", "--dump", "-q", "--std=c99", "--max-configs=1",
                    ] + flags + [source],
                   capture_output=True, text=True, cwd=entry["directory"])
    dump = source + ".dump"
    if not os.path.exists(dump):
        dump = os.path.join(entry["directory"], os.path.basename(source) + ".dump")
    found = collections.defaultdict(set)
    if os.path.exists(dump):
        result = subprocess.run([sys.executable, misra, dump], capture_output=True, text=True)
        for line in (result.stdout + result.stderr).splitlines():
            match = THEIRS.search(line)
            if match:
                found[match.group(3)].add((os.path.basename(match.group(1)), int(match.group(2))))
        os.remove(dump)
    return found


def source_line(files, name, line):
    for path in files:
        if os.path.basename(path) == name and os.path.exists(path):
            with open(path, errors="replace") as handle:
                lines = handle.read().splitlines()
            if 0 < line <= len(lines):
                return lines[line - 1].strip()[:110]
    return ""


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--checker", required=True)
    parser.add_argument("--compile-commands", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--samples", type=int, default=3)
    args = parser.parse_args()

    with open(args.compile_commands) as handle:
        database = json.load(handle)
    misra = find_misra()
    ours, tail = run_ours(args.checker, args.compile_commands)
    theirs = collections.defaultdict(set)
    with tempfile.TemporaryDirectory() as workdir:
        for entry in database:
            for rule, hits in run_theirs(entry, misra, workdir).items():
                theirs[rule] |= hits

    sources = [e["file"] for e in database]
    headers = set()
    for entry in database:
        directory = os.path.dirname(entry["file"])
        for name in os.listdir(directory):
            if name.endswith(".h"):
                headers.add(os.path.join(directory, name))
    searchable = sources + sorted(headers)

    rules = sorted(set(ours) | set(theirs), key=lambda r: [int(p) for p in r.split(".")])
    report = {"checker_summary": tail, "rules": {}}
    for rule in rules:
        o, t = ours.get(rule, set()), theirs.get(rule, set())
        both = o & t
        report["rules"][rule] = {
            "ours": len(o), "cppcheck": len(t), "both": len(both),
            "only_ours": len(o - t), "only_cppcheck": len(t - o),
            "precision_vs_cppcheck": round(len(both) / len(o), 3) if o else None,
            "recall_vs_cppcheck": round(len(both) / len(t), 3) if t else None,
            "samples_only_ours": [
                f"{n}:{l}: {source_line(searchable, n, l)}"
                for n, l in sorted(o - t)[: args.samples]],
            "samples_only_cppcheck": [
                f"{n}:{l}: {source_line(searchable, n, l)}"
                for n, l in sorted(t - o)[: args.samples]],
        }
    with open(args.out, "w") as handle:
        json.dump(report, handle, indent=1)
    print(f"{'rule':>6} {'ours':>6} {'cppchk':>6} {'both':>6} {'prec':>6} {'recall':>6}")
    for rule, row in report["rules"].items():
        p = "-" if row["precision_vs_cppcheck"] is None else f"{row['precision_vs_cppcheck']:.2f}"
        r = "-" if row["recall_vs_cppcheck"] is None else f"{row['recall_vs_cppcheck']:.2f}"
        print(f"{rule:>6} {row['ours']:>6} {row['cppcheck']:>6} {row['both']:>6} {p:>6} {r:>6}")


if __name__ == "__main__":
    main()
