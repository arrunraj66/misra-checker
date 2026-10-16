"""Cross-verify misra-checker findings with cppcheck's MISRA C:2012 add-on.

cppcheck is an independent open implementation of part of MISRA C:2012. It is not ground
truth (the add-on covers only some rules and has its own false positives and false
negatives), so a difference means "one of the two tools should be looked at", not that
either is wrong. The comparison is limited to the rules the add-on implements.

For every translation unit of a compile database cppcheck is run on a symlink inside a
temporary directory (cppcheck writes dump files next to the source; this keeps the
user's folders untouched) and the add-on is run on the dump. Findings are then matched
by (file, line, rule).
"""

from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ADDON_DIRS = ["/usr/lib/x86_64-linux-gnu/cppcheck/addons", "/usr/share/cppcheck/addons",
              "/usr/lib/cppcheck/addons", "/opt/homebrew/share/cppcheck/addons"]
NATIVE = re.compile(r"^(?P<file>[^|]+)\|(?P<line>\d+)\|(?P<severity>\w+)\|(?P<id>[\w.-]+)\|(?P<message>.*)$")
# Diagnostics about cppcheck's own configuration, not about the code.
NATIVE_IGNORED = {"missingInclude", "missingIncludeSystem", "toomanyconfigs", "unmatchedSuppression",
                  "normalCheckLevelMaxBranches", "checkersReport", "noValidConfiguration"}
MAX_NATIVE = 500
THEIRS = re.compile(r"\[(?P<file>[^\]:]+):(?P<line>\d+)\](?P<text>.*)\[misra-c2012-(?P<rule>\d+\.\d+)\]")
STD_MAP = {"c90": "c89", "gnu90": "c89", "c99": "c99", "gnu99": "c99",
           "c11": "c11", "gnu11": "c11", "c17": "c11", "gnu17": "c11"}
PER_FILE_TIMEOUT = 180
TOTAL_TIMEOUT = 900
MAX_UNITS = 300


def find_cppcheck() -> str | None:
    return shutil.which("cppcheck")


def find_misra() -> str | None:
    for directory in ADDON_DIRS:
        candidate = Path(directory) / "misra.py"
        if candidate.is_file():
            return str(candidate)
    return None


def supported_rules(misra: str) -> set[str]:
    """Rules the add-on implements: one `misra_<a>_<b>` check function each."""
    text = Path(misra).read_text(errors="replace")
    return {f"{a}.{b}" for a, b in re.findall(r"def misra_(\d+)_(\d+)\(", text)}


def status() -> dict:
    exe, misra = find_cppcheck(), find_misra()
    if not exe or not misra:
        return {"available": False, "reason": "cppcheck or its MISRA add-on is not installed on the server"}
    try:
        version = subprocess.run([exe, "--version"], capture_output=True, text=True, timeout=15).stdout.strip()
    except (OSError, subprocess.TimeoutExpired):
        return {"available": False, "reason": "cppcheck does not run"}
    return {"available": True, "version": version, "rules_covered": len(supported_rules(misra))}


def flags_from(arguments: list[str]) -> list[str]:
    """Include paths, defines and the language level of one compile command."""
    flags, std, platform = [], "c99", None
    items = list(arguments)
    i = 0
    while i < len(items):
        arg = items[i]
        if arg in ("-I", "-D", "-U") and i + 1 < len(items):
            flags.append(arg + items[i + 1])
            i += 1
        elif arg.startswith(("-I", "-D", "-U")) and len(arg) > 2:
            flags.append(arg)
        elif arg.startswith("-std="):
            std = STD_MAP.get(arg[5:], "c99")
        elif arg.startswith("--target="):
            triple = arg[9:]
            platform = "unix64" if re.match(r"(x86_64|aarch64|riscv64|ppc64|powerpc64|mips64)", triple) else "unix32"
        i += 1
    flags.append(f"--std={std}")
    if platform:
        flags.append(f"--platform={platform}")
    return flags


def parse_native(line: str, unit_dir: str, source: Path, directory: str) -> dict | None:
    m = NATIVE.match(line.strip())
    if not m or m["id"] in NATIVE_IGNORED or m["severity"] == "information":
        return None
    path = m["file"]
    if path.startswith(unit_dir):
        path = str(source)
    elif not os.path.isabs(path):
        path = str(Path(directory or ".") / path)
    return {"file": path, "line": int(m["line"]), "severity": m["severity"], "id": m["id"],
            "message": m["message"].strip()}


def run_unit(index: int, entry: dict, exe: str, misra: str, work: Path,
             deadline: float) -> tuple[list[dict], list[dict], str]:
    """cppcheck + add-on for one translation unit. Returns (misra findings, native findings, error)."""
    source = Path(entry["file"])
    unit_dir = work / str(index)
    unit_dir.mkdir()
    link = unit_dir / source.name
    try:
        link.symlink_to(source)
        arguments = entry.get("arguments") or entry["command"].split()
        proc = subprocess.run(
            [exe, "--dump", "-q", "--max-configs=1", "--language=c", "--enable=warning,style,performance,portability",
             "--template={file}|{line}|{severity}|{id}|{message}", *flags_from(arguments), str(link)],
            capture_output=True, text=True, cwd=entry.get("directory") or str(unit_dir),
            timeout=max(5, min(PER_FILE_TIMEOUT, deadline)))
        dump = Path(str(link) + ".dump")
        if not dump.is_file():
            return [], [], f"{source.name}: cppcheck produced no result ({proc.stderr.strip()[:120]})"
        result = subprocess.run([sys.executable, misra, str(dump)], capture_output=True, text=True,
                                timeout=max(5, min(PER_FILE_TIMEOUT, deadline)))
    except subprocess.TimeoutExpired:
        return [], [], f"{source.name}: cppcheck timed out"
    except OSError as exc:
        return [], [], f"{source.name}: {exc}"
    native = [n for n in (parse_native(l, str(unit_dir), source, entry.get("directory") or "")
                          for l in proc.stderr.splitlines()) if n]
    findings = []
    for line in (result.stdout + result.stderr).splitlines():
        m = THEIRS.search(line)
        if not m:
            continue
        path = m["file"]
        if path.startswith(str(unit_dir)):          # the translation unit itself
            path = str(source)
        elif not os.path.isabs(path):
            path = str(Path(entry.get("directory") or ".") / path)
        findings.append({"file": path, "line": int(m["line"]), "rule": m["rule"],
                         "text": m["text"].strip(" ()")})
    return findings, native, ""


def run(db_entries: list[dict]) -> tuple[list[dict], list[dict], list[str], dict]:
    """Run cppcheck over a compile database. Returns (misra findings, native findings, notes, status)."""
    info = status()
    if not info["available"]:
        raise ValueError(info["reason"])
    exe, misra = find_cppcheck(), find_misra()
    notes = []
    entries = db_entries[:MAX_UNITS]
    if len(db_entries) > MAX_UNITS:
        notes.append(f"cppcheck was run on the first {MAX_UNITS} of {len(db_entries)} files")
    findings: list[dict] = []
    native: list[dict] = []
    import time
    started = time.monotonic()
    with tempfile.TemporaryDirectory(prefix="misra-cppcheck-") as tmp:
        work = Path(tmp)
        with ThreadPoolExecutor(max_workers=min(4, max(1, len(entries)))) as pool:
            futures = [pool.submit(run_unit, i, e, exe, misra, work, TOTAL_TIMEOUT - (time.monotonic() - started))
                       for i, e in enumerate(entries)]
            for future in futures:
                got, got_native, error = future.result()
                findings += got
                native += got_native
                if error:
                    notes.append(error)
    if len(native) > MAX_NATIVE:
        notes.append(f"cppcheck's own checks: showing the first {MAX_NATIVE} of {len(native)}")
        native = native[:MAX_NATIVE]
    return findings, native, notes, info


def compare(ours: list[dict], theirs: list[dict], supported: set[str], prefix: str = "") -> dict:
    """Match findings by (file, line, rule).

    `ours` items need file (already relative), line, rule. `theirs` files are absolute and
    made relative with `prefix`. Annotates every item of `ours` with `cross`:
    "both" | "ours-only" | "not-comparable".
    """
    def relative(path: str) -> str:
        return path[len(prefix):] if prefix and path.startswith(prefix) else path

    theirs_keys = {}
    for t in theirs:
        theirs_keys.setdefault((relative(t["file"]), t["line"], t["rule"]), t)
    our_keys = {(f["file"], f["line"], f["rule"]) for f in ours}
    counts = {"both": 0, "ours-only": 0, "not-comparable": 0}
    for f in ours:
        if f["rule"] not in supported:
            f["cross"] = "not-comparable"
        elif (f["file"], f["line"], f["rule"]) in theirs_keys:
            f["cross"] = "both"
        else:
            f["cross"] = "ours-only"
        counts[f["cross"]] += 1
    cppcheck_only = [
        {"file": file, "line": line, "rule": rule, "message": t["text"], "cross": "cppcheck-only"}
        for (file, line, rule), t in sorted(theirs_keys.items())
        if (file, line, rule) not in our_keys
    ]
    per_rule: dict[str, dict] = {}
    for f in ours:
        if f["rule"] in supported:
            row = per_rule.setdefault(f["rule"], {"both": 0, "ours_only": 0, "cppcheck_only": 0})
            row["both" if f["cross"] == "both" else "ours_only"] += 1
    for f in cppcheck_only:
        per_rule.setdefault(f["rule"], {"both": 0, "ours_only": 0, "cppcheck_only": 0})["cppcheck_only"] += 1
    return {
        "agree": counts["both"], "ours_only": counts["ours-only"], "not_comparable": counts["not-comparable"],
        "cppcheck_only": len(cppcheck_only), "cppcheck_only_findings": cppcheck_only,
        "rules_covered": len(supported),
        "per_rule": {r: per_rule[r] for r in sorted(per_rule, key=lambda x: tuple(map(int, x.split("."))))},
    }


def cross_check(db_entries: list[dict], ours: list[dict], prefix: str = "") -> dict:
    """Run cppcheck on the same files and compare with `ours` (annotated in place)."""
    theirs, native, notes, info = run(db_entries)
    result = compare(ours, theirs, supported_rules(find_misra()), prefix)
    relative = lambda path: path[len(prefix):] if prefix and path.startswith(prefix) else path  # noqa: E731
    our_keys = {(f["file"], f["line"], f["rule"]) for f in ours}
    seen = set()
    result["cppcheck_misra"] = []
    for t in sorted(theirs, key=lambda t: (relative(t["file"]), t["line"], t["rule"])):
        key = (relative(t["file"]), t["line"], t["rule"])
        if key in seen:
            continue
        seen.add(key)
        result["cppcheck_misra"].append({"file": key[0], "line": key[1], "rule": key[2], "message": t["text"],
                                         "also_ours": key in our_keys})
    result["native"] = [{**n, "file": relative(n["file"])} for n in
                        sorted(native, key=lambda n: (relative(n["file"]), n["line"], n["id"]))]
    result["tool"] = info["version"]
    result["notes"] = notes
    return result
