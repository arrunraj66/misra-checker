"""Violation identity, baselines and history.

Professional checkers track a violation by something that survives edits elsewhere in the file,
not by its line number. Here a violation's fingerprint is made of the rule, the file, the text
of the line it is on (whitespace collapsed) and its occurrence number among identical lines of
the same rule in that file. Adding or removing code above it, or reformatting the line, keeps
the fingerprint; changing the line, moving it to another file, or renaming the file does not.

A baseline is a saved list of fingerprints. Comparing a new analysis with it labels every
finding `new` or `unchanged` and lists the baseline entries that no longer occur as `fixed`.
"""

from __future__ import annotations

import hashlib
import json
import re
import threading
import time
from collections import Counter
from pathlib import Path
from typing import Callable

CATEGORY_RANK = {"advisory": 1, "required": 2, "mandatory": 3}
KEEP_SNAPSHOTS = 20


def normalize(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def fingerprint_all(findings: list[dict], line_text: Callable[[str, int], str]) -> None:
    """Add `fp` and `text` to every finding (in place). `line_text(file, line)` returns the source line."""
    seen: Counter = Counter()
    for f in sorted(findings, key=lambda f: (f["file"], f["line"], f.get("col", 0), f["rule"])):
        text = normalize(line_text(f["file"], f["line"]))
        key = (f["rule"], f["file"], text)
        occurrence = seen[key]
        seen[key] += 1
        f["text"] = text[:200]
        f["fp"] = hashlib.sha1(f"{f['rule']}|{f['file']}|{text}|{occurrence}".encode()).hexdigest()[:16]


def entry(f: dict) -> dict:
    return {"fp": f["fp"], "rule": f["rule"], "file": f["file"], "line": f["line"],
            "category": f.get("category", ""), "message": f.get("message", ""), "text": f.get("text", "")}


def compare(current: list[dict], baseline: list[dict]) -> dict:
    """Annotate `current` with status new|unchanged and return the summary and the fixed entries."""
    known = {e["fp"] for e in baseline}
    here = {f["fp"] for f in current}
    counts = Counter()
    gate = Counter()
    for f in current:
        f["status"] = "unchanged" if f["fp"] in known else "new"
        counts[f["status"]] += 1
        if f["status"] == "new":
            gate[f.get("category", "advisory")] += 1
    fixed = [e for e in baseline if e["fp"] not in here]
    return {"new": counts["new"], "unchanged": counts["unchanged"], "fixed": len(fixed),
            "new_by_category": dict(gate), "fixed_entries": fixed}


def worst_new(summary: dict, minimum: str) -> int:
    """How many new findings are at least as severe as `minimum`."""
    floor = CATEGORY_RANK[minimum]
    return sum(n for cat, n in summary["new_by_category"].items() if CATEGORY_RANK.get(cat, 1) >= floor)


def snapshot(findings: list[dict], tool_version: str = "") -> dict:
    by_category = Counter(f.get("category", "") for f in findings)
    return {"saved": time.strftime("%Y-%m-%dT%H:%M:%S%z"), "tool_version": tool_version,
            "counts": {"total": len(findings), **{k: by_category[k] for k in ("mandatory", "required", "advisory")}},
            "findings": [entry(f) for f in findings]}


class BaselineStore:
    """Per user and per workspace key: a history of saved snapshots; the latest is the baseline."""

    def __init__(self, directory: Path) -> None:
        self.dir = directory
        self._lock = threading.Lock()

    def _file(self, user: str, key: str) -> Path:
        slug = re.sub(r"[^A-Za-z0-9_.-]", "_", key)[:80] + "-" + hashlib.sha1(key.encode()).hexdigest()[:8]
        owner = re.sub(r"[^A-Za-z0-9_.@-]", "_", user)[:64]
        return self.dir / owner / f"{slug}.json"

    def _read(self, path: Path) -> dict:
        try:
            return json.loads(path.read_text())
        except (FileNotFoundError, ValueError):
            return {"snapshots": []}

    def latest(self, user: str, key: str) -> dict | None:
        with self._lock:
            snaps = self._read(self._file(user, key)).get("snapshots", [])
        return snaps[-1] if snaps else None

    def history(self, user: str, key: str) -> list[dict]:
        with self._lock:
            snaps = self._read(self._file(user, key)).get("snapshots", [])
        return [{"saved": s["saved"], "tool_version": s.get("tool_version", ""), "counts": s["counts"]} for s in snaps]

    def save(self, user: str, key: str, findings: list[dict], tool_version: str = "") -> dict:
        with self._lock:
            path = self._file(user, key)
            data = self._read(path)
            data["key"] = key
            data.setdefault("snapshots", []).append(snapshot(findings, tool_version))
            data["snapshots"] = data["snapshots"][-KEEP_SNAPSHOTS:]
            path.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
            path.write_text(json.dumps(data, indent=1) + "\n")
            return data["snapshots"][-1]

    def clear(self, user: str, key: str) -> bool:
        with self._lock:
            path = self._file(user, key)
            if path.is_file():
                path.unlink()
                return True
            return False
