#!/usr/bin/env python3
"""Fingerprints, baselines and the CI gate (scripts/baseline.py, scripts/misra_ci.py)."""
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))
import baseline as bl  # noqa: E402

CHECKER, CLANG = sys.argv[1], sys.argv[2]
failures = []


def check(name, condition, detail=""):
    if not condition:
        failures.append(f"{name}: {detail}")


def fps(source, findings):
    lines = source.split("\n")
    items = [dict(f) for f in findings]
    bl.fingerprint_all(items, lambda file, line: lines[line - 1] if 0 < line <= len(lines) else "")
    return items


# ---- fingerprints survive edits elsewhere, reformatting and moving, but not changes to the line
v1 = "int a;\nint x = 010;\nint y = 010;\n"
f1 = fps(v1, [{"file": "m.c", "line": 2, "rule": "7.1"}, {"file": "m.c", "line": 3, "rule": "7.1"}])
v2 = "/* new comment */\nint a;\nint b;\nint x = 010;\nint y = 010;\n"      # code added above
f2 = fps(v2, [{"file": "m.c", "line": 4, "rule": "7.1"}, {"file": "m.c", "line": 5, "rule": "7.1"}])
check("fingerprints survive code added above", [a["fp"] for a in f1] == [a["fp"] for a in f2], (f1, f2))
f3 = fps("int x    =   010;\n", [{"file": "m.c", "line": 1, "rule": "7.1"}])
f4 = fps("int x = 010;\n", [{"file": "m.c", "line": 1, "rule": "7.1"}])
check("whitespace does not matter", f3[0]["fp"] == f4[0]["fp"])
check("identical lines are told apart", f1[0]["fp"] != f1[1]["fp"])
f5 = fps("int x = 011;\n", [{"file": "m.c", "line": 1, "rule": "7.1"}])
check("a changed line is a different violation", f5[0]["fp"] != f4[0]["fp"])
check("another rule on the same line differs", fps("int x = 010;\n", [{"file": "m.c", "line": 1, "rule": "2.2"}])[0]["fp"] != f4[0]["fp"])
check("another file differs", fps("int x = 010;\n", [{"file": "n.c", "line": 1, "rule": "7.1"}])[0]["fp"] != f4[0]["fp"])

# ---- compare
base = [bl.entry(f) for f in f1]
cur = fps("int a;\nint x = 010;\nint z = 077;\n", [{"file": "m.c", "line": 2, "rule": "7.1", "category": "required"},
                                                   {"file": "m.c", "line": 3, "rule": "7.1", "category": "required"}])
summary = bl.compare(cur, base)
check("new / unchanged / fixed", (summary["new"], summary["unchanged"], summary["fixed"]) == (1, 1, 1), summary)
check("statuses are set", {f["line"]: f["status"] for f in cur} == {2: "unchanged", 3: "new"}, cur)
check("fixed entries are reported", summary["fixed_entries"][0]["text"] == "int y = 010;", summary["fixed_entries"])
check("gate counts by category", bl.worst_new(summary, "required") == 1 and bl.worst_new(summary, "mandatory") == 0)

# ---- store: history, latest, isolation per user and key
with tempfile.TemporaryDirectory() as tmp:
    store = bl.BaselineStore(Path(tmp))
    check("no baseline yet", store.latest("bob", "project:fw") is None)
    store.save("bob", "project:fw", cur, "v1")
    store.save("bob", "project:fw", cur[:1], "v2")
    check("latest is the newest snapshot", len(store.latest("bob", "project:fw")["findings"]) == 1)
    check("history keeps counts", [h["counts"]["total"] for h in store.history("bob", "project:fw")] == [2, 1])
    check("other users do not see it", store.latest("erin", "project:fw") is None)
    check("other keys do not see it", store.latest("bob", "project:other") is None)
    check("keys cannot escape the folder", store.save("../../x", "../../y", cur, "v") and not (Path(tmp).parent / "x").exists())
    for _ in range(30):
        store.save("bob", "project:fw", cur, "v")
    check("history is bounded", len(store.history("bob", "project:fw")) == bl.KEEP_SNAPSHOTS)
    check("clear removes it", store.clear("bob", "project:fw") and store.latest("bob", "project:fw") is None)

# ---- the CI gate end to end
with tempfile.TemporaryDirectory() as tmp:
    root = Path(tmp)
    src = root / "m.c"
    db = root / "compile_commands.json"

    def write_db():
        db.write_text(json.dumps([{"directory": str(root), "file": str(src), "arguments": [CLANG, "-std=c99", "-c", str(src)]}]))

    def gate(*extra):
        proc = subprocess.run([sys.executable, str(ROOT / "scripts" / "misra_ci.py"), "--checker", CHECKER,
                               "--compile-commands", str(db), "--source-root", str(root), "--baseline", str(root / "base.json"), *extra],
                              capture_output=True, text=True)
        return proc.returncode, proc.stdout + proc.stderr

    src.write_text("int a;\nint x = 010;\n")
    write_db()
    code, out = gate("--update-baseline")
    check("baseline is written", code == 0 and (root / "base.json").is_file(), out)
    stored = json.loads((root / "base.json").read_text())
    check("baseline paths are relative", all(not f["file"].startswith("/") for f in stored["findings"]), stored["findings"][:2])
    code, out = gate()
    check("unchanged code passes the gate", code == 0 and "0 new" in out, out)
    src.write_text("/* header comment added */\n\nint a;\nint x = 010;\n")   # lines moved, nothing new
    code, out = gate()
    check("moving code does not create new violations", code == 0 and "0 new" in out, out)
    src.write_text("int a;\nint x = 010;\nint y = 077;\n")                          # one new octal constant (required)
    code, out = gate()
    check("a new required violation fails the gate", code == 1 and "GATE FAILED" in out and "int y" not in out and "Rule 7.1" in out, out)
    code, out = gate("--min-category", "mandatory")
    check("the gate threshold is configurable", code == 0, out)
    code, out = gate("--fail-on", "none")
    check("fail-on none never fails", code == 0, out)
    src.write_text("int a;\n")
    code, out = gate()
    check("fixing violations passes and reports them", code == 0 and "0 new," in out and "2 fixed since the baseline" in out, out)
    code, out = subprocess.run([sys.executable, str(ROOT / "scripts" / "misra_ci.py"), "--checker", CHECKER,
                                "--compile-commands", str(root / "missing.json")], capture_output=True, text=True).returncode, ""
    check("a tool failure is exit 2", code == 2, code)

if failures:
    print("\n".join(failures))
    sys.exit(1)
print("baseline tests passed")
