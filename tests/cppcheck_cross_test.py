#!/usr/bin/env python3
"""Matching and flag translation of scripts/cppcheck_cross.py (no cppcheck needed)."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))
import cppcheck_cross as cx  # noqa: E402

failures = []


def check(name, condition, detail=""):
    if not condition:
        failures.append(f"{name}: {detail}")


# ---- flags
flags = cx.flags_from(["clang-14", "-std=gnu99", "--target=arm-none-eabi", "-I", "/p/inc", "-I/p/src", "-DX=1",
                       "-D", "Y", "-UZ", "-isystem", "/sys", "-nostdinc", "-c", "a.c"])
check("includes kept", "-I/p/inc" in flags and "-I/p/src" in flags, flags)
check("defines kept", "-DX=1" in flags and "-DY" in flags and "-UZ" in flags, flags)
check("system include dirs are not passed", "/sys" not in " ".join(flags), flags)
check("language level", "--std=c99" in flags, flags)
check("32-bit target", "--platform=unix32" in flags, flags)
check("64-bit target", "--platform=unix64" in cx.flags_from(["--target=x86_64-linux-gnu"]))
check("c90 maps to c89", "--std=c89" in cx.flags_from(["-std=c90"]))
check("default std", "--std=c99" in cx.flags_from(["-c", "a.c"]))

# ---- compare
ours = [
    {"file": "src/a.c", "line": 10, "rule": "8.4"},     # cppcheck agrees
    {"file": "src/a.c", "line": 20, "rule": "15.1"},    # cppcheck does not report it
    {"file": "src/a.c", "line": 30, "rule": "2.1"},     # rule the add-on does not implement
    {"file": "inc/h.h", "line": 5, "rule": "8.4"},      # header finding, agreed
]
theirs = [
    {"file": "/p/src/a.c", "line": 10, "rule": "8.4", "text": "x"},
    {"file": "/p/src/a.c", "line": 44, "rule": "17.7", "text": "Return value not used"},  # only cppcheck
    {"file": "/p/inc/h.h", "line": 5, "rule": "8.4", "text": "x"},
    {"file": "/p/src/a.c", "line": 10, "rule": "8.4", "text": "duplicate"},
]
supported = {"8.4", "15.1", "17.7"}
result = cx.compare(ours, theirs, supported, "/p/")
check("agree count", result["agree"] == 2, result)
check("ours-only count", result["ours_only"] == 1, result)
check("not comparable count", result["not_comparable"] == 1, result)
check("cppcheck-only count", result["cppcheck_only"] == 1, result)
check("annotations", [f["cross"] for f in ours] == ["both", "ours-only", "not-comparable", "both"], [f["cross"] for f in ours])
only = result["cppcheck_only_findings"][0]
check("cppcheck-only details", (only["file"], only["line"], only["rule"]) == ("src/a.c", 44, "17.7") and only["cross"] == "cppcheck-only", only)
check("per-rule table", result["per_rule"]["8.4"]["both"] == 2 and result["per_rule"]["17.7"]["cppcheck_only"] == 1
      and "2.1" not in result["per_rule"], result["per_rule"])
check("rules covered", result["rules_covered"] == 3)

# nothing reported by either tool
empty = cx.compare([], [], supported, "")
check("empty comparison", empty["agree"] == empty["ours_only"] == empty["cppcheck_only"] == 0, empty)

if failures:
    print("\n".join(failures))
    sys.exit(1)
print("cppcheck cross tests passed")
