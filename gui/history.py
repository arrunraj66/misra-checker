"""Central history: shared projects, runs, trends and API tokens for CI.

Runs (from CI jobs or from GUI users) are stored in SQLite. A run keeps its findings with their
fingerprints (see scripts/baseline.py), so two runs can be compared and every run records how many
violations are new or fixed compared with the previous run of the same branch.

API tokens let a CI job publish without a login. Only a hash of a token is stored; the token itself is
shown once when it is created.
"""

from __future__ import annotations

import hashlib
import json
import os
import re
import secrets
import sqlite3
import threading
import time
from pathlib import Path

NAME_RE = re.compile(r"^[A-Za-z0-9_.-]{1,60}$")
BRANCH_RE = re.compile(r"^[A-Za-z0-9_./-]{1,100}$")
COMMIT_RE = re.compile(r"^[A-Za-z0-9]{0,64}$")
FP_RE = re.compile(r"^[0-9a-f]{16}$")
CATEGORIES = ("mandatory", "required", "advisory")
KEEP_RUNS = 500           # per project and branch
MAX_FINDINGS = 50_000
TOKEN_ROLES = ("viewer", "analyst")

SCHEMA = """
CREATE TABLE IF NOT EXISTS projects(name TEXT PRIMARY KEY, created TEXT, created_by TEXT);
CREATE TABLE IF NOT EXISTS runs(
  id INTEGER PRIMARY KEY AUTOINCREMENT, project TEXT NOT NULL, branch TEXT NOT NULL, commit_id TEXT, created TEXT,
  source TEXT, submitted_by TEXT, tool_version TEXT, total INT, mandatory INT, required INT, advisory INT,
  deviated INT, new_count INT, fixed_count INT, prev_run INT, note TEXT);
CREATE INDEX IF NOT EXISTS runs_by_project ON runs(project, branch, id);
CREATE TABLE IF NOT EXISTS run_findings(
  run_id INT NOT NULL, fp TEXT, rule TEXT, file TEXT, line INT, category TEXT, message TEXT, text TEXT);
CREATE INDEX IF NOT EXISTS findings_by_run ON run_findings(run_id);
"""


class HistoryStore:
    def __init__(self, path: Path) -> None:
        self.path = path
        self._lock = threading.Lock()
        path.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
        with self._connect() as db:
            db.executescript(SCHEMA)
        os.chmod(path, 0o600)

    def _connect(self) -> sqlite3.Connection:
        db = sqlite3.connect(self.path, timeout=30)
        db.row_factory = sqlite3.Row
        db.execute("PRAGMA journal_mode=WAL")
        return db

    # ---- writing
    def add_run(self, *, project: str, branch: str, commit: str, user: str, source: str, tool_version: str,
                findings: list[dict], deviated: int = 0, note: str = "") -> dict:
        if not NAME_RE.match(project):
            raise ValueError("project names use letters, digits, '_', '.' and '-' (max 60)")
        if not BRANCH_RE.match(branch) or ".." in branch or branch.startswith(("/", "-")):
            raise ValueError("bad branch name")
        if not COMMIT_RE.match(commit):
            raise ValueError("bad commit id")
        if len(findings) > MAX_FINDINGS:
            raise ValueError(f"too many findings (limit {MAX_FINDINGS})")
        rows = []
        for f in findings:
            fp, rule = str(f.get("fp", "")), str(f.get("rule", ""))
            category = str(f.get("category", ""))
            if not FP_RE.match(fp) or not re.fullmatch(r"\d+\.\d+", rule) or category not in CATEGORIES:
                raise ValueError("every finding needs a fingerprint, a rule such as 8.4 and a category")
            rows.append((fp, rule, str(f.get("file", ""))[:300], int(f.get("line", 0)), category,
                         str(f.get("message", ""))[:200], str(f.get("text", ""))[:200]))
        counts = {c: sum(1 for r in rows if r[4] == c) for c in CATEGORIES}
        with self._lock, self._connect() as db:
            now = time.strftime("%Y-%m-%dT%H:%M:%S%z")
            db.execute("INSERT OR IGNORE INTO projects(name, created, created_by) VALUES (?,?,?)", (project, now, user))
            prev = db.execute("SELECT id FROM runs WHERE project=? AND branch=? ORDER BY id DESC LIMIT 1", (project, branch)).fetchone()
            prev_fps = set()
            if prev:
                prev_fps = {r[0] for r in db.execute("SELECT fp FROM run_findings WHERE run_id=?", (prev["id"],))}
            here = {r[0] for r in rows}
            new_count = len(here - prev_fps) if prev else 0
            fixed_count = len(prev_fps - here) if prev else 0
            cur = db.execute(
                "INSERT INTO runs(project, branch, commit_id, created, source, submitted_by, tool_version, total, mandatory, required,"
                " advisory, deviated, new_count, fixed_count, prev_run, note) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                (project, branch, commit, now, source, user, tool_version, len(rows), counts["mandatory"], counts["required"],
                 counts["advisory"], int(deviated), new_count, fixed_count, prev["id"] if prev else None, note[:500]))
            run_id = cur.lastrowid
            db.executemany("INSERT INTO run_findings VALUES (?,?,?,?,?,?,?,?)", [(run_id, *r) for r in rows])
            old = db.execute("SELECT id FROM runs WHERE project=? AND branch=? ORDER BY id DESC LIMIT -1 OFFSET ?",
                             (project, branch, KEEP_RUNS)).fetchall()
            for r in old:
                db.execute("DELETE FROM run_findings WHERE run_id=?", (r["id"],))
                db.execute("DELETE FROM runs WHERE id=?", (r["id"],))
            return dict(db.execute("SELECT * FROM runs WHERE id=?", (run_id,)).fetchone())

    def delete_project(self, name: str) -> bool:
        with self._lock, self._connect() as db:
            ids = [r[0] for r in db.execute("SELECT id FROM runs WHERE project=?", (name,))]
            for i in ids:
                db.execute("DELETE FROM run_findings WHERE run_id=?", (i,))
            db.execute("DELETE FROM runs WHERE project=?", (name,))
            return db.execute("DELETE FROM projects WHERE name=?", (name,)).rowcount > 0

    # ---- reading
    def projects(self) -> list[dict]:
        with self._lock, self._connect() as db:
            out = []
            for p in db.execute("SELECT * FROM projects ORDER BY name"):
                last = db.execute("SELECT * FROM runs WHERE project=? ORDER BY id DESC LIMIT 1", (p["name"],)).fetchone()
                count = db.execute("SELECT COUNT(*) FROM runs WHERE project=?", (p["name"],)).fetchone()[0]
                out.append({"name": p["name"], "created": p["created"], "created_by": p["created_by"], "runs": count,
                            "latest": dict(last) if last else None,
                            "branches": [r[0] for r in db.execute("SELECT DISTINCT branch FROM runs WHERE project=? ORDER BY branch", (p["name"],))]})
            return out

    def runs(self, project: str, branch: str | None = None, limit: int = 100) -> list[dict]:
        with self._lock, self._connect() as db:
            if branch:
                rows = db.execute("SELECT * FROM runs WHERE project=? AND branch=? ORDER BY id DESC LIMIT ?", (project, branch, limit)).fetchall()
            else:
                rows = db.execute("SELECT * FROM runs WHERE project=? ORDER BY id DESC LIMIT ?", (project, limit)).fetchall()
            return [dict(r) for r in reversed(rows)]

    def run(self, run_id: int, with_findings: bool = True) -> dict | None:
        with self._lock, self._connect() as db:
            r = db.execute("SELECT * FROM runs WHERE id=?", (run_id,)).fetchone()
            if not r:
                return None
            out = dict(r)
            if with_findings:
                out["findings"] = [dict(f) for f in db.execute(
                    "SELECT fp, rule, file, line, category, message, text FROM run_findings WHERE run_id=? ORDER BY file, line LIMIT 5000", (run_id,))]
            return out

    def diff(self, older: int, newer: int) -> dict | None:
        a, b = self.run(older), self.run(newer)
        if not a or not b or a["project"] != b["project"]:
            return None
        fa, fb = {f["fp"]: f for f in a["findings"]}, {f["fp"]: f for f in b["findings"]}
        return {"older": {k: v for k, v in a.items() if k != "findings"}, "newer": {k: v for k, v in b.items() if k != "findings"},
                "new": [fb[k] for k in fb if k not in fa], "fixed": [fa[k] for k in fa if k not in fb],
                "unchanged": sum(1 for k in fb if k in fa)}


class TokenStore:
    """API tokens for CI. Stored as SHA-256 hashes; the token is shown once on creation."""

    def __init__(self, path: Path) -> None:
        self.path = path
        self._lock = threading.Lock()

    def _load(self) -> dict:
        try:
            return json.loads(self.path.read_text())
        except FileNotFoundError:
            return {}

    def _save(self, data: dict) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
        tmp = self.path.with_suffix(".tmp")
        fd = os.open(tmp, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
        with os.fdopen(fd, "w") as handle:
            json.dump(data, handle, indent=2)
        os.replace(tmp, self.path)

    @staticmethod
    def _hash(token: str) -> str:
        return hashlib.sha256(token.encode()).hexdigest()

    def create(self, name: str, role: str) -> str:
        if not NAME_RE.match(name):
            raise ValueError("token names use letters, digits, '_', '.' and '-'")
        if role not in TOKEN_ROLES:
            raise ValueError(f"a token's role must be one of: {', '.join(TOKEN_ROLES)}")
        token = "mt_" + secrets.token_urlsafe(32)
        with self._lock:
            data = self._load()
            if any(rec["name"] == name for rec in data.values()):
                raise ValueError("a token with that name already exists")
            data[self._hash(token)] = {"name": name, "role": role, "created": time.strftime("%Y-%m-%dT%H:%M:%S%z"), "last_used": ""}
            self._save(data)
        return token

    def verify(self, token: str) -> dict | None:
        if not token.startswith("mt_") or len(token) > 100:
            return None
        with self._lock:
            data = self._load()
            record = data.get(self._hash(token))
            if record:
                record["last_used"] = time.strftime("%Y-%m-%dT%H:%M:%S%z")
                self._save(data)
                return dict(record)
        return None

    def revoke(self, name: str) -> bool:
        with self._lock:
            data = self._load()
            keep = {h: r for h, r in data.items() if r["name"] != name}
            if len(keep) == len(data):
                return False
            self._save(keep)
            return True

    def list(self) -> list[dict]:
        with self._lock:
            return sorted(self._load().values(), key=lambda r: r["name"])
