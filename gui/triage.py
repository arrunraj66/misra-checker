"""Triage store for the GUI: verdicts on findings (true / false positive) and missed violations
(false negatives), kept as replayable cases. See scripts/triage_runner.py for the case format.
"""

from __future__ import annotations

import hashlib
import io
import json
import re
import threading
import time
import uuid
import zipfile
from pathlib import Path

KINDS = ("false-positive", "false-negative", "true-positive")
SNAPSHOT_LIMIT = 400_000     # bytes of source kept per case
MAX_NOTE = 2000
ID_RE = re.compile(r"^[a-f0-9]{12}$")


class TriageStore:
    def __init__(self, directory: Path) -> None:
        self.dir = directory
        self._lock = threading.Lock()

    def _path(self, case_id: str) -> Path:
        if not (ID_RE.match(case_id) or re.fullmatch(r"[A-Za-z0-9_.-]{1,60}", case_id)):
            raise ValueError("bad case id")
        return self.dir / f"{case_id}.json"

    def _all(self) -> list[dict]:
        cases = []
        for path in sorted(self.dir.glob("*.json")) if self.dir.is_dir() else []:
            try:
                cases.append(json.loads(path.read_text()))
            except ValueError:
                continue
        return cases

    def visible(self, user: str, role: str) -> list[dict]:
        with self._lock:
            return [c for c in self._all() if role == "admin" or c.get("user") == user]

    def get(self, case_id: str, user: str, role: str) -> dict | None:
        with self._lock:
            path = self._path(case_id)
            if not path.is_file():
                return None
            case = json.loads(path.read_text())
        return case if (role == "admin" or case.get("user") == user) else None

    def save(self, case: dict) -> dict:
        with self._lock:
            self.dir.mkdir(parents=True, exist_ok=True, mode=0o700)
            # The same verdict on the same line text is updated, not duplicated.
            fingerprint = hashlib.sha1("|".join([case["user"], case["kind"], case["rule"], case["file"],
                                                 case["line_text"]]).encode()).hexdigest()
            case["fingerprint"] = fingerprint
            for existing in self._all():
                if existing.get("fingerprint") == fingerprint:
                    case["id"] = existing["id"]
                    break
            else:
                case["id"] = uuid.uuid4().hex[:12]
            self._path(case["id"]).write_text(json.dumps(case, indent=1) + "\n")
        return case

    def update_state(self, case_id: str, state: str) -> None:
        with self._lock:
            path = self._path(case_id)
            if path.is_file():
                case = json.loads(path.read_text())
                case["state"] = state
                path.write_text(json.dumps(case, indent=1) + "\n")

    def delete(self, case_id: str, user: str, role: str) -> bool:
        with self._lock:
            path = self._path(case_id)
            if not path.is_file():
                return False
            case = json.loads(path.read_text())
            if role != "admin" and case.get("user") != user:
                return False
            path.unlink()
            return True

    def export_zip(self, user: str, role: str) -> bytes:
        buffer = io.BytesIO()
        with zipfile.ZipFile(buffer, "w", zipfile.ZIP_DEFLATED) as archive:
            for case in self.visible(user, role):
                archive.writestr(f"{case['id']}.json", json.dumps(case, indent=1) + "\n")
        return buffer.getvalue()


def summarize(cases: list[dict]) -> dict:
    """Per rule: counts of each verdict and the precision the verdicts suggest."""
    rules: dict[str, dict] = {}
    for case in cases:
        row = rules.setdefault(case["rule"], {"true_positive": 0, "false_positive": 0, "false_negative": 0})
        row[case["kind"].replace("-", "_")] += 1
    for row in rules.values():
        judged = row["true_positive"] + row["false_positive"]
        row["precision"] = round(row["true_positive"] / judged, 3) if judged else None
    return {r: rules[r] for r in sorted(rules, key=lambda x: tuple(map(int, x.split("."))))}


def public(case: dict) -> dict:
    """The case without its source snapshot (for lists)."""
    return {k: v for k, v in case.items() if k != "files"} | {"file_count": len(case.get("files", {}))}


def make_case(*, user: str, kind: str, rule: str, file: str, line: int, note: str, files: dict[str, str],
              build: dict, tool_version: str, compiler: str, mode: str) -> dict:
    if kind not in KINDS:
        raise ValueError("kind must be false-positive, false-negative or true-positive")
    if not re.fullmatch(r"\d+\.\d+", rule):
        raise ValueError("bad rule id")
    if file not in files:
        raise ValueError("the file is not part of the analysed source")
    lines = files[file].split("\n")
    if not (1 <= line <= len(lines)):
        raise ValueError("the line is outside the file")
    return {"id": "", "kind": kind, "rule": rule, "file": file, "line": line,
            "line_text": lines[line - 1].strip()[:300], "note": note[:MAX_NOTE], "user": user,
            "created": time.strftime("%Y-%m-%dT%H:%M:%S%z"), "state": "open", "mode": mode,
            "compiler": compiler, "tool_version": tool_version, "build": build, "files": files}
