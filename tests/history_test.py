#!/usr/bin/env python3
"""gui/history.py: run history, diffs, retention and API tokens."""
import os
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "gui"))
import history as hs  # noqa: E402

failures = []


def check(name, condition, detail=""):
    if not condition:
        failures.append(f"{name}: {detail}")


def f(n, rule="8.4", cat="required", file="a.c", line=1):
    return {"fp": f"{n:016x}", "rule": rule, "category": cat, "file": file, "line": line, "message": "m", "text": "t"}


with tempfile.TemporaryDirectory() as tmp:
    store = hs.HistoryStore(Path(tmp) / "history.db")
    check("database file is private", oct(os.stat(Path(tmp) / "history.db").st_mode & 0o777) == "0o600")

    def add(findings, **kw):
        return store.add_run(project=kw.get("project", "fw"), branch=kw.get("branch", "main"), commit=kw.get("commit", "abc123"),
                             user="ci", source="ci", tool_version="v1", findings=findings, deviated=kw.get("deviated", 0))

    r1 = add([f(1), f(2, "7.1"), f(3, "19.2", "advisory")])
    check("first run has nothing to compare with", (r1["new_count"], r1["fixed_count"], r1["prev_run"]) == (0, 0, None), r1)
    check("counts by category", (r1["total"], r1["mandatory"], r1["required"], r1["advisory"]) == (3, 0, 2, 1), r1)
    r2 = add([f(1), f(2, "7.1"), f(4, "9.1", "mandatory")])
    check("second run: one new, one fixed", (r2["new_count"], r2["fixed_count"], r2["prev_run"]) == (1, 1, r1["id"]), r2)
    r3 = add([f(1)], branch="feature/x")
    check("another branch compares only with its own history", r3["prev_run"] is None and r3["new_count"] == 0, r3)
    r4 = add([f(1), f(5)])
    check("main continues from its last main run", r4["prev_run"] == r2["id"] and r4["fixed_count"] == 2 and r4["new_count"] == 1, r4)

    runs = store.runs("fw", "main")
    check("runs are oldest first for charts", [r["id"] for r in runs] == [r1["id"], r2["id"], r4["id"]], [r["id"] for r in runs])
    check("all branches listed when none is given", len(store.runs("fw")) == 4)
    d = store.diff(r1["id"], r2["id"])
    check("diff lists new and fixed findings", [x["rule"] for x in d["new"]] == ["9.1"] and [x["rule"] for x in d["fixed"]] == ["19.2"] and d["unchanged"] == 2, d)
    other = add([f(1)], project="other")
    check("runs of different projects cannot be diffed", store.diff(r1["id"], other["id"]) is None)
    check("unknown run", store.run(99999) is None)
    check("run detail carries findings", len(store.run(r2["id"])["findings"]) == 3)
    listing = {p["name"]: p for p in store.projects()}
    check("projects are listed with their latest run", listing["fw"]["runs"] == 4 and listing["fw"]["latest"]["id"] == r4["id"]
          and listing["fw"]["branches"] == ["feature/x", "main"], listing["fw"])

    for name, kwargs in {"bad project": dict(project="a b"), "bad branch": dict(branch="../x"), "bad commit": dict(commit="x y")}.items():
        try:
            add([f(1)], **kwargs)
            check(f"{name} rejected", False)
        except ValueError:
            pass
    for bad in ({"fp": "zz", "rule": "8.4", "category": "required"}, {"fp": f(1)["fp"], "rule": "x", "category": "required"},
                {"fp": f(1)["fp"], "rule": "8.4", "category": "severe"}):
        try:
            add([bad])
            check(f"invalid finding rejected: {bad}", False)
        except ValueError:
            pass
    try:
        add([f(i) for i in range(hs.MAX_FINDINGS + 1)])
        check("too many findings rejected", False)
    except ValueError:
        pass

    old = hs.KEEP_RUNS
    hs.KEEP_RUNS = 3
    try:
        for i in range(6):
            add([f(i)], project="busy")
        kept = store.runs("busy")
        check("old runs are dropped past the retention limit", len(kept) == 3, len(kept))
    finally:
        hs.KEEP_RUNS = old
    check("deleting a project removes its runs", store.delete_project("busy") and store.runs("busy") == [] and not store.delete_project("busy"))

    # ---- tokens
    tokens = hs.TokenStore(Path(tmp) / "tokens.json")
    token = tokens.create("ci-main", "analyst")
    check("token has the expected shape", token.startswith("mt_") and len(token) > 40, token)
    check("token verifies", tokens.verify(token)["role"] == "analyst" and tokens.verify(token)["name"] == "ci-main")
    check("only a hash is stored", token not in (Path(tmp) / "tokens.json").read_text())
    check("file is private", oct(os.stat(Path(tmp) / "tokens.json").st_mode & 0o777) == "0o600")
    check("wrong token", tokens.verify("mt_nope") is None and tokens.verify("") is None and tokens.verify(token + "x") is None)
    check("usage is recorded", tokens.list()[0]["last_used"] != "")
    for name, role in (("ci-main", "viewer"), ("x y", "analyst"), ("admin-tok", "admin")):
        try:
            tokens.create(name, role)
            check(f"token ({name}, {role}) rejected", False)
        except ValueError:
            pass
    check("revoking works", tokens.revoke("ci-main") and tokens.verify(token) is None and not tokens.revoke("ci-main"))

if failures:
    print("\n".join(failures))
    sys.exit(1)
print("history tests passed")
