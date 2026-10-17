"""Compliance documents in the shape MISRA Compliance:2020 asks for, generated from an analysis.

* Guideline Compliance Summary: for every rule, whether the analysis found no violation, only
  deviated violations, or unresolved violations (or could not assess it).
* Deviation register: every deviation record, whether it is in use, expired or unused, and what
  it covers.
* Guideline Enforcement Plan: how each rule is enforced (which tool, how reliable, what is left to
  review).
* The evidence behind the tool: versions, compiler profile, scope of the analysis, validation status.

The documents describe what the tool found. They never state that a project "is MISRA compliant":
that judgement belongs to the project, needs the directives and the process evidence this tool cannot
see, and needs an independent review of the tool itself.
"""

from __future__ import annotations

import csv
import html
import io
import json
import re
import time
from collections import Counter

DISCLAIMER = (
    "This report lists what misra-checker found in the analysed sources. It is not a statement that the "
    "software complies with MISRA C:2012. Compliance needs every guideline to be compliant or covered by an "
    "approved deviation, the directives and process evidence this tool does not check, and a tool whose "
    "detectors have been independently validated or qualified. The detectors have so far been checked against "
    "the project's own fixtures and one open-source oracle only (see docs/independent-validation.md)."
)
CATEGORY_ORDER = {"mandatory": 0, "required": 1, "advisory": 2}


def rule_key(rule: str) -> tuple[int, int]:
    a, b = rule.split(".")
    return int(a), int(b)


def deviation_status(record: dict, used: int, today: str) -> str:
    if record.get("expires") and record["expires"] < today:
        return "expired"
    return "in use" if used else "unused"


def build(ctx: dict) -> dict:
    """Combine the analysis context into the three documents (plain data, rendered separately)."""
    today = ctx.get("date") or time.strftime("%Y-%m-%d")
    result = ctx["result"]
    rules = sorted(ctx["rules"], key=lambda r: rule_key(r["id"]))
    active = Counter(f["rule"] for f in result["findings"])
    deviated = Counter(f["rule"] for f in result["deviated"])
    not_assessed = set(ctx.get("not_assessed", []))
    validation = ctx.get("validation", {})
    cross = result.get("cross") or {}
    cppcheck_rules = set(ctx.get("cppcheck_rules", []))

    summary = []
    for r in rules:
        rid = r["id"]
        if rid in not_assessed:
            status, detail = "not assessed", "needs at least two translation units analysed together"
        elif active[rid]:
            status, detail = "violations", f"{active[rid]} unresolved"
        elif deviated[rid]:
            status, detail = "deviated", f"{deviated[rid]} covered by deviation"
        else:
            status, detail = "no violation reported", ""
        summary.append({
            "rule": rid, "category": r["category"].lower(), "topic": r["topic"], "decidability": r["decidability"],
            "status": status, "detail": detail, "violations": active[rid], "deviated": deviated[rid],
            "fixture_validation": validation.get(rid, "not run"),
        })

    # --- deviation register
    used = Counter(f.get("deviation", "") for f in result["deviated"])
    where: dict[str, list[str]] = {}
    for f in result["deviated"]:
        where.setdefault(f.get("deviation", ""), []).append(f"{f['file']}:{f['line']} (Rule {f['rule']})")
    register = []
    for d in ctx.get("deviations", []):
        register.append({
            "id": d.get("id", ""), "rules": d.get("rules", []), "file": d.get("file", "") or "(any file)",
            "lines": d.get("lines"), "justification": d.get("justification", ""), "approved_by": d.get("approved_by", ""),
            "expires": d.get("expires", ""), "status": deviation_status(d, used[d.get("id", "")], today),
            "findings_covered": used[d.get("id", "")], "locations": where.get(d.get("id", ""), [])[:50],
        })

    # --- enforcement plan
    plan = []
    for row in summary:
        rid = row["rule"]
        tools = ["misra-checker (static analysis)"]
        if rid in cppcheck_rules:
            tools.append("cppcheck MISRA add-on (cross-check)" if cross else "cppcheck MISRA add-on (available, not run)")
        remaining = []
        if row["decidability"].lower().startswith("undecid"):
            remaining.append("undecidable: the tool approximates, complement with review")
        if row["fixture_validation"] not in ("PASS",):
            remaining.append(f"fixture validation {row['fixture_validation']}")
        if rid in not_assessed:
            remaining.append("analyse the whole program to assess")
        plan.append({"rule": rid, "category": row["category"], "enforced_by": tools, "remaining": remaining})

    counts = Counter(row["status"] for row in summary)
    unresolved = {c: sum(1 for f in result["findings"] if f["category"] == c) for c in CATEGORY_ORDER}
    blocking = unresolved["mandatory"] + unresolved["required"]
    verdict = ("No unresolved mandatory or required violation was reported by misra-checker for the assessed rules."
               if not blocking and not not_assessed else
               f"{blocking} unresolved mandatory or required violation(s) reported." if blocking else
               "No unresolved mandatory or required violation was reported, but "
               f"{len(not_assessed)} rule(s) could not be assessed.")
    return {
        "title": f"MISRA C:2012 analysis report: {ctx.get('project', 'workspace')}",
        "date": today, "disclaimer": DISCLAIMER, "scope": ctx.get("scope", {}), "tools": ctx.get("tools", {}),
        "verdict": verdict, "unresolved": unresolved, "status_counts": dict(counts),
        "summary": summary, "register": register, "plan": plan,
        "violations": sorted(result["findings"], key=lambda f: (CATEGORY_ORDER.get(f["category"], 3), rule_key(f["rule"]), f["file"], f["line"])),
        "baseline": result.get("baseline"), "validation_counts": dict(Counter(validation.values())),
        "oracle": ctx.get("oracle"),
    }


# ----------------------------------------------------------------------------------------- renderers

def to_csv(doc: dict) -> str:
    buf = io.StringIO()
    w = csv.writer(buf)
    w.writerow(["rule", "category", "topic", "status", "unresolved_violations", "deviated_violations", "decidability", "fixture_validation"])
    for r in doc["summary"]:
        w.writerow([r["rule"], r["category"], r["topic"], r["status"], r["violations"], r["deviated"], r["decidability"], r["fixture_validation"]])
    return buf.getvalue()


def to_json(doc: dict) -> str:
    return json.dumps(doc, indent=1)


STYLE = """
body{font:14px/1.5 system-ui,sans-serif;margin:32px auto;max-width:1100px;color:#171c24;padding:0 16px}
h1{font-size:22px;margin:0 0 4px}h2{font-size:17px;margin:28px 0 8px;border-bottom:1px solid #dde2e9;padding-bottom:4px}
table{border-collapse:collapse;width:100%;font-size:13px}th,td{border:1px solid #dde2e9;padding:5px 8px;text-align:left;vertical-align:top}
th{background:#f3f5f8}.m{font-family:ui-monospace,monospace;font-size:12px}.note{color:#5d6775;font-size:12.5px}
.box{border:1px solid #dde2e9;border-left:4px solid #2457d6;background:#f8f9fb;padding:10px 14px;margin:12px 0}
.violations{color:#c62a35;font-weight:600}.deviated{color:#16794c}.not-assessed{color:#946200}
.mandatory{color:#c62a35}.required{color:#b45309}.advisory{color:#2457d6}
@media print{body{margin:12px}h2{break-after:avoid}tr{break-inside:avoid}}
"""


def _e(x) -> str:
    return html.escape(str(x))


def to_html(doc: dict) -> str:
    out = [f"<!doctype html><html lang='en'><head><meta charset='utf-8'><title>{_e(doc['title'])}</title><style>{STYLE}</style></head><body>"]
    out.append(f"<h1>{_e(doc['title'])}</h1><div class='note'>Generated {_e(doc['date'])}</div>")
    out.append(f"<div class='box'><b>Result.</b> {_e(doc['verdict'])}<br><span class='note'>{_e(doc['disclaimer'])}</span></div>")

    out.append("<h2>1. Scope and tools</h2><table>")
    for k, v in {**doc["scope"], **{f"Tool: {t}": ver for t, ver in doc["tools"].items()}}.items():
        out.append(f"<tr><th style='width:30%'>{_e(k)}</th><td>{_e(v)}</td></tr>")
    out.append("</table>")

    u = doc["unresolved"]
    out.append("<h2>2. Result summary</h2><table><tr><th>Mandatory</th><th>Required</th><th>Advisory</th><th>Guidelines with violations</th>"
               "<th>Deviated</th><th>No violation reported</th><th>Not assessed</th></tr><tr>"
               f"<td class='mandatory'>{u['mandatory']}</td><td class='required'>{u['required']}</td><td class='advisory'>{u['advisory']}</td>"
               f"<td>{doc['status_counts'].get('violations', 0)}</td><td>{doc['status_counts'].get('deviated', 0)}</td>"
               f"<td>{doc['status_counts'].get('no violation reported', 0)}</td><td>{doc['status_counts'].get('not assessed', 0)}</td></tr></table>")
    if doc.get("baseline"):
        b = doc["baseline"]
        out.append(f"<p class='note'>Compared with the baseline saved {_e(b.get('saved', ''))}: {b['new']} new, {b['unchanged']} unchanged, {b['fixed']} fixed.</p>")

    out.append("<h2>3. Guideline Compliance Summary</h2><table><tr><th>Rule</th><th>Category</th><th>Topic</th><th>Status</th><th>Detail</th><th>Fixture validation</th></tr>")
    for r in doc["summary"]:
        cls = r["status"].replace(" ", "-")
        out.append(f"<tr><td class='m'>{_e(r['rule'])}</td><td class='{_e(r['category'])}'>{_e(r['category'])}</td><td>{_e(r['topic'])}</td>"
                   f"<td class='{_e(cls)}'>{_e(r['status'])}</td><td>{_e(r['detail'])}</td><td>{_e(r['fixture_validation'])}</td></tr>")
    out.append("</table><p class='note'>Directives (Dir 1.1 and following) are not checked by this tool; they need process and review evidence.</p>")

    out.append("<h2>4. Unresolved violations</h2>")
    if doc["violations"]:
        out.append("<table><tr><th>Category</th><th>Rule</th><th>Location</th><th>Check</th><th>Line</th></tr>")
        for f in doc["violations"]:
            out.append(f"<tr><td class='{_e(f['category'])}'>{_e(f['category'])}</td><td class='m'>{_e(f['rule'])}</td>"
                       f"<td class='m'>{_e(f['file'])}:{_e(f['line'])}</td><td>{_e(f.get('message', ''))}</td><td class='m'>{_e(f.get('text', ''))}</td></tr>")
        out.append("</table>")
    else:
        out.append("<p>None.</p>")

    out.append("<h2>5. Deviation register</h2>")
    if doc["register"]:
        out.append("<table><tr><th>Id</th><th>Rules</th><th>Scope</th><th>Justification</th><th>Approved by</th><th>Expires</th><th>Status</th><th>Covers</th></tr>")
        for d in doc["register"]:
            scope = d["file"] + (f", lines {d['lines'][0]}-{d['lines'][1]}" if d.get("lines") else "")
            out.append(f"<tr><td class='m'>{_e(d['id'])}</td><td>{_e(', '.join(d['rules']))}</td><td>{_e(scope)}</td><td>{_e(d['justification'])}</td>"
                       f"<td>{_e(d['approved_by'])}</td><td>{_e(d['expires'] or 'never')}</td><td>{_e(d['status'])}</td>"
                       f"<td>{d['findings_covered']}" + (f"<div class='note'>{_e('; '.join(d['locations'][:5]))}</div>" if d["locations"] else "") + "</td></tr>")
        out.append("</table><p class='note'>Mandatory rules cannot be deviated. Expired deviations are not applied; unused ones should be reviewed.</p>")
    else:
        out.append("<p>No deviation records were supplied.</p>")

    out.append("<h2>6. Guideline Enforcement Plan</h2><table><tr><th>Rule</th><th>Category</th><th>Enforced by</th><th>Left to review or open</th></tr>")
    for p in doc["plan"]:
        out.append(f"<tr><td class='m'>{_e(p['rule'])}</td><td class='{_e(p['category'])}'>{_e(p['category'])}</td>"
                   f"<td>{_e('; '.join(p['enforced_by']))}</td><td>{_e('; '.join(p['remaining']) or '—')}</td></tr>")
    out.append("</table>")

    out.append("<h2>7. Evidence and limits</h2><ul>")
    vc = doc["validation_counts"]
    out.append(f"<li>Per-rule fixture validation: {_e(', '.join(f'{k} {v}' for k, v in sorted(vc.items())) or 'not run')}. "
               "Fixtures were written by the tool's authors.</li>")
    oracle = doc.get("oracle")
    out.append("<li>Independent validation: " + (f"one open-source oracle ({_e(oracle['name'])}), where {oracle['rules_with_findings']} rules produced any finding"
                                                   if oracle else "no oracle result available")
               + "; no independent reviewer yet.</li>")
    out.append("<li>The tool is not qualified or certified for use in a safety process.</li></ul>")
    out.append("</body></html>")
    return "".join(out)
