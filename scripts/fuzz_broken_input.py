#!/usr/bin/env python3
"""Robustness check: analyze fixtures with injected syntax/semantic errors.

The checker must keep running on code that does not compile (no crash, no hang)
so that one compile error does not hide every other finding. Exit status 1 if
any run is killed by a signal or times out.
"""
import json, random, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CHECKER = ROOT / "build" / "misra-checker"
CLANG = "/usr/bin/clang-14"
MUTATIONS = [
    lambda l: l.replace(";", "", 1),
    lambda l: l.replace("(", "", 1),
    lambda l: l.replace(")", "", 1),
    lambda l: l.replace("{", "", 1),
    lambda l: l.replace("}", "", 1),
    lambda l: l + " undeclared_symbol + 1;",
    lambda l: l.replace("int", "intt", 1),
    lambda l: l.replace("=", "= = ", 1),
    lambda l: l.replace("return", "retur", 1),
    lambda l: l.replace(",", ",,", 1),
    lambda l: "",
]


def run(src: str, tmp: Path) -> int:
    f = tmp / "fuzz.c"
    f.write_text(src)
    db = tmp / "db.json"
    db.write_text(json.dumps([{"directory": str(tmp), "file": str(f),
                               "arguments": [CLANG, "-std=c99", "-c", str(f)]}]))
    try:
        return subprocess.run([str(CHECKER), "analyze", "--compile-commands", str(db)],
                              capture_output=True, timeout=60).returncode
    except subprocess.TimeoutExpired:
        return -999


def main() -> int:
    rng = random.Random(int(sys.argv[1]) if len(sys.argv) > 1 else 1)
    rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 3
    files = sorted((ROOT / "tests" / "fixtures" / "obs").glob("*.c"))
    bad, runs = [], 0
    with tempfile.TemporaryDirectory() as t:
        tmp = Path(t)
        for path in files:
            lines = path.read_text().split("\n")
            for _ in range(rounds):
                mutated = list(lines)
                i = rng.randrange(len(mutated))
                mutated[i] = rng.choice(MUTATIONS)(mutated[i])
                src = "\n".join(mutated)
                code = run(src, tmp)
                runs += 1
                if code < 0 or code >= 100:
                    bad.append((path.name, code, src))
    print(f"{runs} runs, {len(bad)} crashes/timeouts")
    for name, code, src in bad[:10]:
        print(f"--- {name}: exit {code}\n{src}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
