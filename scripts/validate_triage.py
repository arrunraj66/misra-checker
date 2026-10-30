#!/usr/bin/env python3
"""Run triage cases (see triage_runner.py).

    validate_triage.py [DIR ...] [--check] [--update]

DIR defaults to tests/triage. With --check the exit status is 1 when a case marked `fixed` no
longer behaves as labelled (a regression); open cases are listed but do not fail. With --update
the `state` field of each case file is rewritten from the result.
"""
import argparse
import json
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import triage_runner as tr  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("dirs", nargs="*", type=Path)
    parser.add_argument("--checker", default=str(ROOT / "build" / "misra-checker"))
    parser.add_argument("--clang", default=shutil.which("clang-14") or shutil.which("clang") or "clang")
    parser.add_argument("--check", action="store_true", help="fail when a fixed case regresses")
    parser.add_argument("--update", action="store_true", help="rewrite the state of each case file")
    args = parser.parse_args()

    cases = [c for d in (args.dirs or [ROOT / "tests" / "triage"]) if d.is_dir() for c in tr.load_cases(d)]
    regressions, counts = [], {"fixed": 0, "open": 0, "skipped": 0}
    for case in cases:
        result = tr.run_case(args.checker, args.clang, case)
        counts[result["state"]] += 1
        was = case.get("state", "open")
        flag = ""
        if result["state"] == "skipped":
            flag = "skipped"
        elif was == "fixed" and result["state"] == "open":
            flag = "REGRESSION"
            regressions.append(case["id"])
        elif was == "open" and result["state"] == "fixed":
            flag = "now fixed (run with --update)"
        print(f"{case['id']:<28} {case['kind']:<15} rule {case['rule']:<6} {case['file']}:{case['line']:<5} "
              f"{result['state']:<8} {result['detail']} {flag}")
        if args.update and result["state"] != "skipped":
            path = Path(case["_path"])
            data = json.loads(path.read_text())
            data["state"] = result["state"]
            path.write_text(json.dumps(data, indent=1) + "\n")
    print(f"\n{len(cases)} case(s): {counts['fixed']} as labelled, {counts['open']} open, {counts['skipped']} skipped")
    if regressions:
        print("regressions: " + ", ".join(regressions))
    return 1 if (args.check and regressions) else 0


if __name__ == "__main__":
    sys.exit(main())
