#!/usr/bin/env python3
"""Validate every MISRA rule, one at a time, against its fixtures.

For each rule the script finds the fixtures that exercise it:

* ``bad``  fixtures must make the checker report that rule (exit status 1 and a
  ``misra-c2012-<rule>-`` diagnostic);
* ``ok``   fixtures must not report that rule and must not make the checker fail.

A rule is PASS when all its cases behave as expected and it has at least one bad
and one ok case, PARTIAL when it passes but lacks one side, FAIL when any case
misbehaves, and UNCOVERED when it has no fixtures at all.

This is a regression check of the detectors against the repository's own
fixtures. It is not independent validation or a conformance claim.
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
from dataclasses import asdict, dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Fixtures that predate the rX_Y_{bad,ok} naming scheme: rule -> (kind, path).
LEGACY_CASES = {
    "15.1": [("bad", "rule_15_1/violating.c"), ("ok", "rule_15_1/compliant.c")],
    "15.2": [("bad", "rule_15_2/backward.c"), ("ok", "rule_15_2/forward.c")],
    "15.3": [("bad", "rule_15_3/into_block.c"), ("bad", "rule_15_3/sibling_block.c"),
             ("ok", "rule_15_3/compliant.c")],
    "15.4": [("bad", "rule_15_4/two_breaks.c"), ("bad", "rule_15_4/break_and_goto.c"),
             ("ok", "rule_15_4/compliant.c")],
}

# Rules whose violating construct cannot be written in a program that Clang
# accepts, so no meaningful bad fixture can exist.
NO_BAD_POSSIBLE = {
    "11.7": "pointer<->float casts are a Clang compile error",
}

NAME_RE = re.compile(r"^r(\d+)_(\d+)(?:_[a-z0-9]+)?_(bad|ok)(?:\.c)?$")


@dataclass
class CaseResult:
    name: str
    kind: str  # "bad" | "ok"
    passed: bool
    exit_code: int
    detail: str
    findings: list[str] = field(default_factory=list)


@dataclass
class RuleResult:
    rule: str
    status: str  # PASS | PARTIAL | FAIL | UNCOVERED
    note: str
    cases: list[CaseResult]


def find_checker() -> Path:
    for candidate in (ROOT / "build" / "misra-checker", ROOT / "build" / "bin" / "misra-checker"):
        if candidate.exists():
            return candidate
    sys.exit("misra-checker not found; build the project first (ninja -C build)")


def find_clang() -> str:
    for name in ("clang-14", "clang", "gcc", "cc"):
        path = shutil.which(name)
        if path:
            return path
    sys.exit("no C compiler found for the compile database")


def list_rules(checker: Path) -> list[str]:
    out = subprocess.run([str(checker), "--list-rules"], capture_output=True, text=True, check=True)
    return out.stdout.split()


def discover_cases() -> dict[str, list[tuple[str, str, list[Path]]]]:
    """rule -> [(case name, kind, [source files])]."""
    fixtures = ROOT / "tests" / "fixtures"
    cases: dict[str, list[tuple[str, str, list[Path]]]] = {}
    for path in sorted((fixtures / "obs").glob("*.c")):
        m = NAME_RE.match(path.name)
        if m:
            cases.setdefault(f"{m[1]}.{m[2]}", []).append((path.stem, m[3], [path]))
    for path in sorted((fixtures / "sys").iterdir()):
        m = NAME_RE.match(path.name)
        if m and path.is_dir():
            sources = sorted(path.glob("*.c"))
            if sources:
                cases.setdefault(f"{m[1]}.{m[2]}", []).append((path.name, m[3], sources))
    for rule, entries in LEGACY_CASES.items():
        for kind, rel in entries:
            path = fixtures / rel
            cases.setdefault(rule, []).append((rel.replace("/", ":"), kind, [path]))
    return cases


def run_case(checker: Path, compiler: str, rule: str, name: str, kind: str,
             sources: list[Path]) -> CaseResult:
    database = [
        {"directory": str(src.parent), "file": str(src),
         "arguments": [compiler, "-std=c99", "-c", str(src)]}
        for src in sources
    ]
    with tempfile.TemporaryDirectory(prefix="misra-validate-") as tmp:
        db_path = Path(tmp) / "compile_commands.json"
        db_path.write_text(json.dumps(database))
        proc = subprocess.run(
            [str(checker), "analyze", "--compile-commands", str(db_path)],
            capture_output=True, text=True, timeout=120)
    pattern = f"misra-c2012-{rule}-"
    lines = [ln for ln in proc.stdout.splitlines() if pattern in ln]
    if kind == "bad":
        ok = proc.returncode == 1 and bool(lines)
        detail = "reported" if ok else f"rule not reported (exit {proc.returncode})"
    else:
        ok = not lines and proc.returncode in (0, 1)
        if lines:
            detail = "unexpected finding"
        elif proc.returncode not in (0, 1):
            detail = f"checker failed (exit {proc.returncode}): {proc.stderr.strip()[:200]}"
        else:
            detail = "no finding"
    return CaseResult(name, kind, ok, proc.returncode, detail, lines)


def validate_rule(checker: Path, compiler: str, rule: str, cases) -> RuleResult:
    results = [run_case(checker, compiler, rule, *case) for case in cases.get(rule, [])]
    if not results:
        return RuleResult(rule, "UNCOVERED", "no fixtures", [])
    kinds = {r.kind for r in results}
    if not all(r.passed for r in results):
        return RuleResult(rule, "FAIL", "; ".join(f"{r.name}: {r.detail}" for r in results if not r.passed), results)
    if kinds == {"bad", "ok"}:
        return RuleResult(rule, "PASS", "", results)
    missing = "bad" if "bad" not in kinds else "ok"
    note = f"no {missing} fixture"
    if missing == "bad" and rule in NO_BAD_POSSIBLE:
        note += f" ({NO_BAD_POSSIBLE[rule]})"
    return RuleResult(rule, "PARTIAL", note, results)


def rule_sort_key(rule: str) -> tuple[int, int]:
    a, b = rule.split(".")
    return int(a), int(b)


def validate(rules: list[str] | None = None) -> list[RuleResult]:
    checker, compiler = find_checker(), find_clang()
    cases = discover_cases()
    selected = sorted(rules or list_rules(checker), key=rule_sort_key)
    return [validate_rule(checker, compiler, rule, cases) for rule in selected]


def render_markdown(results: list[RuleResult]) -> str:
    counts: dict[str, int] = {}
    for r in results:
        counts[r.status] = counts.get(r.status, 0) + 1
    lines = ["# Per-rule fixture validation", "",
             "Regression of each detector against its bad/ok fixtures. Not independent validation.", "",
             "Summary: " + ", ".join(f"{k} {v}" for k, v in sorted(counts.items())), "",
             "| Rule | Status | Bad | Ok | Note |", "|---|---|---|---|---|"]
    for r in results:
        bad = [c for c in r.cases if c.kind == "bad"]
        okc = [c for c in r.cases if c.kind == "ok"]
        lines.append(f"| {r.rule} | {r.status} | {sum(c.passed for c in bad)}/{len(bad)} "
                     f"| {sum(c.passed for c in okc)}/{len(okc)} | {r.note} |")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("rules", nargs="*", help="rule ids such as 8.4 (default: all)")
    parser.add_argument("--json", type=Path, help="write full results as JSON")
    parser.add_argument("--markdown", type=Path, help="write a summary table as Markdown")
    parser.add_argument("-q", "--quiet", action="store_true", help="only print problems and the summary")
    args = parser.parse_args()

    results = validate(args.rules or None)
    for r in results:
        if args.quiet and r.status in ("PASS",):
            continue
        print(f"Rule {r.rule:<6} {r.status:<9} {r.note}")
        if not args.quiet:
            for c in r.cases:
                print(f"    [{'ok ' if c.passed else 'ERR'}] {c.kind:<3} {c.name}: {c.detail}")
    counts: dict[str, int] = {}
    for r in results:
        counts[r.status] = counts.get(r.status, 0) + 1
    print("\n" + ", ".join(f"{k}: {v}" for k, v in sorted(counts.items())) + f" (of {len(results)})")
    if args.json:
        args.json.write_text(json.dumps([asdict(r) for r in results], indent=2))
    if args.markdown:
        args.markdown.write_text(render_markdown(results))
    return 1 if counts.get("FAIL") else 0


if __name__ == "__main__":
    sys.exit(main())
