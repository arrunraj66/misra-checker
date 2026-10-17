#!/usr/bin/env python3
"""scripts/compliance.py: statuses, deviation register, enforcement plan and rendering."""
import csv
import io
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))
import compliance as cm  # noqa: E402

failures = []


def check(name, condition, detail=""):
    if not condition:
        failures.append(f"{name}: {detail}")


rules = [
    {"id": "7.1", "category": "Required", "decidability": "Decidable", "topic": "Constants"},
    {"id": "8.4", "category": "Required", "decidability": "Decidable", "topic": "Declarations"},
    {"id": "19.2", "category": "Advisory", "decidability": "Decidable", "topic": "Unions"},
    {"id": "9.1", "category": "Mandatory", "decidability": "Undecidable", "topic": "Initialization"},
    {"id": "8.6", "category": "Required", "decidability": "Decidable", "topic": "Definitions"},
    {"id": "2.1", "category": "Required", "decidability": "Undecidable", "topic": "Unreachable code"},
]
finding = lambda rule, line, cat="required": {"rule": rule, "file": "src/a.c", "line": line, "category": cat, "message": "m", "text": "int x;"}  # noqa: E731
result = {
    "findings": [finding("7.1", 3), finding("7.1", 9), finding("9.1", 12, "mandatory")],
    "deviated": [dict(finding("19.2", 5, "advisory"), deviation="DEV-1"), dict(finding("19.2", 6, "advisory"), deviation="DEV-1")],
    "baseline": {"saved": "2026-10-01", "new": 1, "unchanged": 2, "fixed": 0},
    "cross": {"tool": "Cppcheck 2.7"},
}
deviations = [
    {"id": "DEV-1", "rules": ["19.2"], "file": "src/a.c", "justification": "register overlay", "approved_by": "lead", "expires": "2099-01-01"},
    {"id": "DEV-2", "rules": ["8.4"], "justification": "legacy", "approved_by": "lead", "expires": "2020-01-01"},
    {"id": "DEV-3", "rules": ["2.1"], "justification": "unused", "approved_by": "lead"},
]
doc = cm.build({"project": "fw.zip", "date": "2026-10-17", "result": result, "rules": rules, "deviations": deviations,
                "not_assessed": ["8.6"], "validation": {"7.1": "PASS", "8.4": "PASS", "19.2": "PASS", "9.1": "PARTIAL", "8.6": "PASS", "2.1": "PASS"},
                "cppcheck_rules": ["7.1", "8.4"], "scope": {"Files": "2"}, "tools": {"misra-checker": "0.4.0"}})
status = {r["rule"]: r for r in doc["summary"]}
check("violations", status["7.1"]["status"] == "violations" and status["7.1"]["violations"] == 2, status["7.1"])
check("deviated only", status["19.2"]["status"] == "deviated" and status["19.2"]["deviated"] == 2, status["19.2"])
check("no violation reported", status["8.4"]["status"] == "no violation reported", status["8.4"])
check("not assessed wins over silence", status["8.6"]["status"] == "not assessed", status["8.6"])
check("never says compliant", all("compliant" not in r["status"] for r in doc["summary"]))
check("rules are in numeric order", [r["rule"] for r in doc["summary"]] == ["2.1", "7.1", "8.4", "8.6", "9.1", "19.2"], [r["rule"] for r in doc["summary"]])
reg = {d["id"]: d for d in doc["register"]}
check("deviation in use", reg["DEV-1"]["status"] == "in use" and reg["DEV-1"]["findings_covered"] == 2, reg["DEV-1"])
check("deviation expired", reg["DEV-2"]["status"] == "expired", reg["DEV-2"])
check("deviation unused", reg["DEV-3"]["status"] == "unused", reg["DEV-3"])
check("deviation lists where it applies", "src/a.c:5 (Rule 19.2)" in reg["DEV-1"]["locations"], reg["DEV-1"]["locations"])
plan = {p["rule"]: p for p in doc["plan"]}
check("plan names the cross-check tool", any("cppcheck" in t for t in plan["7.1"]["enforced_by"]) and len(plan["19.2"]["enforced_by"]) == 1, plan["7.1"])
check("plan flags undecidable and partial rules", any("undecidable" in r for r in plan["9.1"]["remaining"]) and any("PARTIAL" in r for r in plan["9.1"]["remaining"]), plan["9.1"])
check("plan says what is needed for a not assessed rule", any("whole program" in r for r in plan["8.6"]["remaining"]), plan["8.6"])
check("unresolved counts by category", doc["unresolved"] == {"mandatory": 1, "required": 2, "advisory": 0}, doc["unresolved"])
check("verdict reports blocking violations", "3 unresolved" in doc["verdict"], doc["verdict"])
clean = cm.build({"project": "x", "result": {"findings": [], "deviated": []}, "rules": rules, "validation": {}})
check("clean verdict is careful", "No unresolved" in clean["verdict"] and "compliant" not in clean["verdict"].lower(), clean["verdict"])
partial = cm.build({"project": "x", "result": {"findings": [], "deviated": []}, "rules": rules, "not_assessed": ["8.6"], "validation": {}})
check("clean but not everything assessed says so", "could not be assessed" in partial["verdict"], partial["verdict"])
check("the disclaimer is always present", "not a statement that the software complies" in doc["disclaimer"])

page = cm.to_html(doc)
for needle in ("Guideline Compliance Summary", "Deviation register", "Guideline Enforcement Plan", "DEV-1", "register overlay", "Unresolved violations"):
    check(f"html has: {needle}", needle in page)
check("html is self-contained", "<script" not in page and "http://" not in page and "https://" not in page)
hostile = cm.to_html(cm.build({"project": "<script>alert(1)</script>", "result": {"findings": [], "deviated": []}, "rules": rules, "validation": {},
                               "deviations": [{"id": "<img src=x>", "rules": ["7.1"], "justification": "<b>x</b>", "approved_by": "a"}]}))
check("untrusted text is escaped", "<script>alert" not in hostile and "<img src=x>" not in hostile and "<b>x</b>" not in hostile)
rows = list(csv.DictReader(io.StringIO(cm.to_csv(doc))))
check("csv has one row per rule", len(rows) == 6 and rows[1]["rule"] == "7.1" and rows[1]["unresolved_violations"] == "2", rows[:2])
check("json round trips", json.loads(cm.to_json(doc))["summary"][0]["rule"] == "2.1")

if failures:
    print("\n".join(failures))
    sys.exit(1)
print("compliance tests passed")
