#!/usr/bin/env python3
"""SARIF output and deviation workflow of `misra-checker analyze`."""
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

CHECKER, CLANG, FIXTURE = sys.argv[1], sys.argv[2], Path(sys.argv[3])
failures = []


def check(name, condition, detail=""):
    if not condition:
        failures.append(f"{name}: {detail}")


with tempfile.TemporaryDirectory() as tmp:
    tmp = Path(tmp)
    src = tmp / "octal.c"
    src.write_text((FIXTURE / "r7_1_bad.c").read_text())
    db = tmp / "compile_commands.json"
    db.write_text(json.dumps([{"directory": str(tmp), "file": str(src),
                               "arguments": [CLANG, "-std=c99", "-c", str(src)]}]))

    def run(*extra, env=None):
        return subprocess.run([CHECKER, "analyze", "--compile-commands", str(db), *extra],
                              capture_output=True, text=True,
                              env={**os.environ, **(env or {})})

    def deviations(entries):
        path = tmp / "dev.json"
        path.write_text(json.dumps({"deviations": entries}))
        return str(path)

    base = run()
    check("baseline exit", base.returncode == 1, base.stdout)
    check("baseline finding", "Rule 7.1" in base.stdout)

    # --format sarif: stdout is only the document
    sarif = run("--format", "sarif", "--source-root", str(tmp))
    doc = json.loads(sarif.stdout)
    run0 = doc["runs"][0]
    check("sarif version", doc["version"] == "2.1.0")
    result = next(r for r in run0["results"] if r["ruleId"] == "misra-c2012-7.1")
    check("sarif level", result["level"] == "warning", result["level"])
    loc = result["locations"][0]["physicalLocation"]
    check("sarif uri relative", loc["artifactLocation"] == {"uri": "octal.c", "uriBaseId": "SRCROOT"}, loc)
    check("sarif line", loc["region"]["startLine"] >= 1)
    check("sarif rule table", any(r["id"] == "misra-c2012-7.1" for r in run0["tool"]["driver"]["rules"]))
    check("sarif active has no suppression", "suppressions" not in result)

    # a deviation covering the finding
    dev = deviations([{"id": "DEV-1", "rules": ["7.1"], "file": "octal.c",
                       "justification": "reviewed", "approved_by": "lead"}])
    covered = run("--deviations", dev, "--format", "sarif")
    sup = json.loads(covered.stdout)["runs"][0]["results"]
    hit = [r for r in sup if r["ruleId"] == "misra-c2012-7.1"]
    check("deviation suppresses in sarif", hit and hit[0]["suppressions"][0]["justification"] == "reviewed", sup)
    text = run("--deviations", dev)
    check("deviated finding is not an active finding", "required: MISRA C:2012 Rule 7.1" not in text.stdout, text.stdout)
    check("deviated line printed", "deviated: MISRA C:2012 Rule 7.1" in text.stdout and "(DEV-1)" in text.stdout)

    # line scope that misses the finding keeps it active and reports the record unused
    miss = run("--deviations", deviations([{"id": "DEV-2", "rules": ["7.1"], "lines": [900, 901],
                                            "justification": "j", "approved_by": "a"}]))
    check("line scope miss stays active", "required: MISRA C:2012 Rule 7.1" in miss.stdout)
    check("unused reported", "DEV-2 matched no finding" in miss.stderr, miss.stderr)

    # glob and expiry
    glob = run("--deviations", deviations([{"id": "DEV-3", "rules": ["7.1"], "file": "*.c",
                                            "justification": "j", "approved_by": "a", "expires": "2030-01-01"}]),
               env={"MISRA_TODAY": "2029-12-31"})
    check("glob match before expiry", "(DEV-3)" in glob.stdout, glob.stdout)
    late = run("--deviations", deviations([{"id": "DEV-3", "rules": ["7.1"], "file": "*.c",
                                            "justification": "j", "approved_by": "a", "expires": "2030-01-01"}]),
               env={"MISRA_TODAY": "2030-01-02"})
    check("expired not applied", "required: MISRA C:2012 Rule 7.1" in late.stdout, late.stdout)
    check("expired reported", "DEV-3 has expired" in late.stderr)

    # invalid files are fatal (exit 65) before analysis
    for name, entry, needle in [
        ("mandatory", {"id": "M", "rules": ["9.1"], "justification": "j", "approved_by": "a"}, "mandatory"),
        ("no justification", {"id": "N", "rules": ["7.1"], "approved_by": "a"}, "justification"),
        ("no approver", {"id": "N", "rules": ["7.1"], "justification": "j"}, "approved_by"),
        ("unknown rule", {"id": "U", "rules": ["99.9"], "justification": "j", "approved_by": "a"}, "unknown rule"),
        ("bad date", {"id": "D", "rules": ["7.1"], "justification": "j", "approved_by": "a", "expires": "tomorrow"}, "YYYY-MM-DD"),
    ]:
        bad = run("--deviations", deviations([entry]))
        check(f"invalid ({name}) exit 65", bad.returncode == 65, f"{bad.returncode} {bad.stderr}")
        check(f"invalid ({name}) message", needle in bad.stderr, bad.stderr)

    # --sarif-out writes the file while text goes to stdout
    out = tmp / "out.sarif"
    both = run("--sarif-out", str(out))
    check("sarif-out file", json.loads(out.read_text())["version"] == "2.1.0")
    check("sarif-out keeps text", "Rule 7.1" in both.stdout)

if failures:
    print("\n".join(failures))
    sys.exit(1)
print("report tests passed")
