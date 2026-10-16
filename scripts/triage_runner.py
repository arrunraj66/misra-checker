"""Replay triage cases (labelled findings) against the current misra-checker.

A case is a JSON file written by the GUI (or by hand) that records a verdict about one
rule on one line of some code, together with everything needed to reproduce the analysis:

    {"id": "...", "kind": "false-positive" | "false-negative" | "true-positive",
     "rule": "17.4", "file": "src/main.c", "line": 12, "note": "...",
     "files": {"src/main.c": "<source>", ...},
     "build": {"std": "c99", "clang_flags": [...], "defines": [...], "include_dirs": ["", "inc"]},
     "state": "open" | "fixed"}

* false-positive : the checker must NOT report `rule` at `file:line`.
* false-negative : the checker MUST report `rule` at `file:line` (a violation it missed).
* true-positive  : the checker MUST report it (guards against regressions).

A case whose expectation holds is `fixed` (the checker behaves as labelled); otherwise it is
`open` (a known defect or gap). Promoted cases live in tests/triage and are run by ctest:
a fixed case that starts failing is a regression; open cases are listed but do not fail.
"""

from __future__ import annotations

import json
import re
import subprocess
import tempfile
from pathlib import Path

KINDS = ("false-positive", "false-negative", "true-positive")
FINDING = re.compile(r"^(?P<file>.+?):(?P<line>\d+):(?P<col>\d+): (?P<cat>\w+): MISRA C:2012 Rule (?P<rule>\S+) \[")
MAX_UNITS = 40


def load_cases(directory: Path) -> list[dict]:
    cases = []
    for path in sorted(Path(directory).glob("*.json")):
        try:
            case = json.loads(path.read_text())
        except ValueError:
            continue
        if case.get("kind") in KINDS and case.get("files"):
            case["_path"] = str(path)
            cases.append(case)
    return cases


def run_case(checker: str, clang: str, case: dict, timeout: int = 120) -> dict:
    """Returns {"ok": bool, "state": "fixed"|"open"|"skipped", "detail": str}."""
    build = case.get("build", {})
    flags = list(build.get("clang_flags", []))
    for i, flag in enumerate(flags):  # a compiler profile that is not installed here cannot be replayed
        if flag == "-isystem" and i + 1 < len(flags) and not Path(flags[i + 1]).is_dir():
            return {"ok": True, "state": "skipped", "detail": f"compiler profile needs {flags[i + 1]}"}
    with tempfile.TemporaryDirectory(prefix="misra-triage-") as tmp:
        root = Path(tmp).resolve()
        for name, text in case["files"].items():
            target = (root / name).resolve()
            if root not in target.parents:
                return {"ok": False, "state": "open", "detail": f"unsafe path {name}"}
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(text)
        include_dirs = build.get("include_dirs") or [""]
        common = [f"-std={build.get('std', 'c99')}", *flags, *(f"-D{d}" for d in build.get("defines", [])),
                  *(f"-I{root / d if d else root}" for d in include_dirs)]
        units = sorted(n for n in case["files"] if n.endswith(".c"))[:MAX_UNITS]
        if not units:
            return {"ok": False, "state": "open", "detail": "the case has no .c file"}
        database = [{"directory": str(root), "file": str(root / u), "arguments": [clang, *common, "-c", str(root / u)]}
                    for u in units]
        (root / "compile_commands.json").write_text(json.dumps(database))
        proc = subprocess.run([checker, "analyze", "--compile-commands", str(root / "compile_commands.json")],
                              capture_output=True, text=True, timeout=timeout)
        prefix = str(root) + "/"
        hit = False
        for line in proc.stdout.splitlines():
            m = FINDING.match(line)
            if m and m["cat"] != "deviated" and m["rule"] == case["rule"] \
                    and m["file"].replace(prefix, "") == case["file"] and int(m["line"]) == int(case["line"]):
                hit = True
                break
    want_hit = case["kind"] != "false-positive"
    ok = hit == want_hit
    if case["kind"] == "false-positive":
        detail = "no longer reported" if ok else "still reported"
    else:
        detail = "reported" if ok else "not reported"
    return {"ok": ok, "state": "fixed" if ok else "open", "detail": detail}
