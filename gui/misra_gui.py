#!/usr/bin/env python3
"""Web GUI for misra-checker.

* analyzes pasted/uploaded C code, zip projects, or an existing compile database,
* applies deviation records and exports SARIF,
* lists all rules with their per-rule fixture validation status and validates them.

Only the Python standard library is used. Access requires a login (roles:
viewer, analyst, admin) unless --no-auth is given, which is only allowed when
bound to loopback. Manage users with --add-user / --remove-user / --list-users.

    python3 gui/misra_gui.py --add-user alice --role analyst
    python3 gui/misra_gui.py [--port 8765] [--data-dir DIR] [--tls-cert C --tls-key K]
"""

from __future__ import annotations

import argparse
import io
import json
import os
import re
import secrets
import ssl
import subprocess
import sys
import tempfile
import time
import uuid
import webbrowser
import zipfile
from collections import OrderedDict
from dataclasses import asdict
from http.cookies import SimpleCookie
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))
sys.path.insert(0, str(Path(__file__).resolve().parent))
import auth  # noqa: E402
import validate_rules as vr  # noqa: E402

INDEX = Path(__file__).resolve().parent / "index.html"
MAX_BODY = 8 * 1024 * 1024
MAX_ZIP = 100 * 1024 * 1024
MAX_UNPACKED = 300 * 1024 * 1024
MAX_PROJECT_FILES = 10000
MAX_PROJECTS_PER_USER = 3
MAX_DEVIATION_BYTES = 1024 * 1024
DEVIATION_FILE = "misra-deviations.json"
COOKIE = "misra_session"
LIBRARY_DIR = re.compile(
    r"(^|/)(libs?|librar(y|ies)|third[_-]?party|vendors?|externa?l?|drivers?|hal|cmsis|middlewares?|sdk|thirdparty)(/|$)",
    re.I)
DEFINE_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*(=[^\s]*)?$")
FINDING_RE = re.compile(
    r"^(?P<file>.+?):(?P<line>\d+):(?P<col>\d+): (?P<category>\w+): "
    r"MISRA C:2012 Rule (?P<rule>\S+) \[(?P<key>[^\]]+)\](?: \((?P<deviation>[^)]+)\))?$")
STANDARDS = {"c90", "c99", "c11", "c17"}
SAFE_NAME = re.compile(r"^[A-Za-z0-9_][A-Za-z0-9_.-]{0,63}\.[ch]$")

# Minimum role per route (see auth.ROLES).
ROUTE_ROLE = {
    ("GET", "/api/rules"): "viewer", ("GET", "/api/samples"): "viewer",
    ("GET", "/api/validate"): "analyst", ("GET", "/api/validate-all"): "analyst",
    ("GET", "/api/file"): "analyst", ("POST", "/api/analyze"): "analyst",
    ("POST", "/api/project"): "analyst",
}


class State:
    projects: "OrderedDict[str, Project]" = OrderedDict()
    checker: Path
    compiler: str
    cases: dict
    users: auth.UserStore
    sessions = auth.Sessions()
    limiter = auth.LoginLimiter()
    audit = auth.AuditLog(None)
    require_auth = True
    secure_cookie = False
    allowed_hosts = {"127.0.0.1", "localhost", "::1"}
    db_roots: list[Path] = []


def load_rule_table() -> list[dict]:
    """Rule metadata from the engineering inventory (no normative text)."""
    rows = []
    path = ROOT / "docs" / "rule-implementation-status.md"
    for line in path.read_text().splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) == 8 and re.fullmatch(r"\d+\.\d+", cells[0]):
            rows.append({"id": cells[0], "category": cells[1], "decidability": cells[2],
                         "scope": cells[3], "topic": cells[6]})
    return rows


def rule_category(rule: str) -> str:
    if not hasattr(rule_category, "table"):
        rule_category.table = {r["id"]: r["category"].lower() for r in load_rule_table()}
    return rule_category.table.get(rule, "advisory")


def parse_findings(stdout: str) -> tuple[list[dict], list[dict], str]:
    findings, deviated, summary = [], [], []
    for line in stdout.splitlines():
        m = FINDING_RE.match(line)
        if m:
            d = m.groupdict()
            d["line"], d["col"] = int(d["line"]), int(d["col"])
            d["message"] = re.sub(r"^misra-c2012-\d+\.\d+-", "", d["key"]).replace("-", " ")
            if d["category"] == "deviated":
                d["category"] = rule_category(d["rule"])
                if d not in deviated:
                    deviated.append(d)
            else:
                d.pop("deviation")
                if d not in findings:
                    findings.append(d)
        elif line.strip():
            summary.append(line.strip())
    return findings, deviated, " ".join(summary)


def run_checker(db_path: Path, timeout: int = 300, deviations: str = "",
                source_root: str = "") -> dict:
    if len(deviations.encode()) > MAX_DEVIATION_BYTES:
        raise ValueError("deviation file is too large")
    with tempfile.TemporaryDirectory(prefix="misra-run-") as run_dir:
        args = [str(State.checker), "analyze", "--compile-commands", str(db_path),
                "--sarif-out", str(Path(run_dir) / "result.sarif")]
        if deviations.strip():
            dev_path = Path(run_dir) / "deviations.json"
            dev_path.write_text(deviations)
            args += ["--deviations", str(dev_path)]
        if source_root:
            args += ["--source-root", source_root]
        started = time.monotonic()
        proc = subprocess.run(args, capture_output=True, text=True, timeout=timeout)
        elapsed_ms = round((time.monotonic() - started) * 1000)
        sarif = ""
        sarif_file = Path(run_dir) / "result.sarif"
        if sarif_file.is_file():
            document = json.loads(sarif_file.read_text())
            for run in document.get("runs", []):
                run.pop("originalUriBaseIds", None)  # do not leak server paths
            sarif = json.dumps(document)
    findings, deviated, summary = parse_findings(proc.stdout)
    compile_errors = "results may be incomplete" in proc.stderr
    notes = [ln.replace("misra-checker: ", "") for ln in proc.stderr.splitlines()
             if "deviation" in ln and ("expired" in ln or "matched no finding" in ln)]
    error = ""
    if proc.returncode not in (0, 1) and not compile_errors:
        error = proc.stderr.strip()
    return {"exit_code": proc.returncode, "findings": findings, "deviated": deviated,
            "summary": summary, "elapsed_ms": elapsed_ms, "compile_errors": compile_errors,
            "compiler_output": proc.stderr.strip()[:20000] if compile_errors else "",
            "deviation_notes": notes, "error": error, "sarif": sarif}


def analyze_files(files: list[dict], std: str, deviations: str) -> dict:
    """Analyze several in-memory files together (enables whole-program rules)."""
    if std not in STANDARDS:
        raise ValueError("unsupported language standard")
    if not files or len(files) > 50:
        raise ValueError("provide between 1 and 50 files")
    names = [str(f.get("name", "")) for f in files]
    if any(not SAFE_NAME.match(n) for n in names) or len(set(names)) != len(names):
        raise ValueError("file names must be unique and look like name.c or name.h")
    if not any(n.endswith(".c") for n in names):
        raise ValueError("at least one .c file is required")
    with tempfile.TemporaryDirectory(prefix="misra-gui-") as tmp:
        for f in files:
            (Path(tmp) / f["name"]).write_text(str(f.get("code", "")))
        db = Path(tmp) / "compile_commands.json"
        db.write_text(json.dumps([{
            "directory": tmp, "file": str(Path(tmp) / n),
            "arguments": [State.compiler, f"-std={std}", "-I", tmp, "-c", str(Path(tmp) / n)]}
            for n in names if n.endswith(".c")]))
        result = run_checker(db, deviations=deviations, source_root=tmp)
        prefix = tmp + "/"
    for key in ("compiler_output", "error"):
        result[key] = result[key].replace(prefix, "")
    for f in result["findings"] + result["deviated"]:
        f["file"] = f["file"].replace(prefix, "")
    return result


def analyze_database(path: str, deviations: str) -> dict:
    p = Path(path).expanduser().resolve()
    if p.is_dir():
        p = p / "compile_commands.json"
    if not State.db_roots:
        raise ValueError("analyzing server-side paths is disabled; start the server with --db-root DIR")
    if not any(root == p or root in p.parents for root in State.db_roots):
        raise ValueError("that path is outside the allowed --db-root directories")
    if not p.is_file():
        raise ValueError(f"no such compile database: {p}")
    return run_checker(p, deviations=deviations)


def case_with_source(case: vr.CaseResult, sources: list[Path]) -> dict:
    d = asdict(case)
    d["files"] = [{"name": s.name, "source": s.read_text(errors="replace")} for s in sources]
    return d


def validate_one(rule: str, with_source: bool) -> dict:
    result = vr.validate_rule(State.checker, State.compiler, rule, State.cases)
    out = asdict(result)
    if with_source:
        by_name = {name: srcs for name, _k, srcs in State.cases.get(rule, [])}
        out["cases"] = [case_with_source(c, by_name[c.name]) for c in result.cases]
    return out


class Project:
    """A zip uploaded by a user, unpacked (C sources and headers only) in a temp dir."""

    def __init__(self, owner: str, name: str, tmp: tempfile.TemporaryDirectory, root: Path,
                 c_files: list[str], h_files: list[str], deviations: str) -> None:
        self.id = uuid.uuid4().hex
        self.owner, self.name, self._tmp, self.root = owner, name, tmp, root
        self.c_files, self.h_files, self.deviations = c_files, h_files, deviations

    @property
    def include_dirs(self) -> list[str]:
        dirs = {""}
        for rel in self.c_files + self.h_files:
            dirs.add(str(Path(rel).parent) if str(Path(rel).parent) != "." else "")
        return sorted(dirs)[:300]

    def describe(self) -> dict:
        return {"id": self.id, "name": self.name, "c_files": len(self.c_files),
                "h_files": len(self.h_files), "include_dirs": len(self.include_dirs),
                "library_files": sum(1 for f in self.c_files + self.h_files if LIBRARY_DIR.search(f)),
                "has_deviations": bool(self.deviations.strip()),
                "files": [{"path": f, "kind": "c" if f.endswith(".c") else "h",
                           "library": bool(LIBRARY_DIR.search(f))}
                          for f in sorted(self.c_files + self.h_files)]}


def create_project(owner: str, name: str, data: bytes) -> Project:
    try:
        archive = zipfile.ZipFile(io.BytesIO(data))
    except zipfile.BadZipFile as exc:
        raise ValueError("not a valid zip archive") from exc
    wanted, deviation_info = [], None
    for info in archive.infolist():
        path = info.filename.replace("\\", "/")
        parts = [p for p in path.split("/") if p not in ("", ".")]
        if info.is_dir() or not parts or "__MACOSX" in parts:
            continue
        if path.startswith("/") or ".." in parts or ":" in parts[0]:
            raise ValueError(f"unsafe path in archive: {info.filename}")
        if ((info.external_attr >> 16) & 0o170000) == 0o120000:
            continue  # symlink
        if parts[-1].lower().endswith((".c", ".h")):
            wanted.append((info, "/".join(parts)))
        elif parts[-1] == DEVIATION_FILE and len(parts) <= 2:
            deviation_info = info
    if not wanted:
        raise ValueError("the archive contains no .c or .h files")
    if len(wanted) > MAX_PROJECT_FILES or sum(i.file_size for i, _ in wanted) > MAX_UNPACKED:
        raise ValueError("project is too large")
    prefix = ""  # drop a single common top-level folder
    firsts = {rel.split("/")[0] for _, rel in wanted}
    if len(firsts) == 1 and all("/" in rel for _, rel in wanted):
        prefix = firsts.pop() + "/"
    tmp = tempfile.TemporaryDirectory(prefix="misra-project-")
    root = Path(tmp.name).resolve()
    c_files, h_files = [], []
    for info, rel in wanted:
        rel = rel[len(prefix):] if prefix else rel
        target = (root / rel).resolve()
        if root not in target.parents:
            raise ValueError(f"unsafe path in archive: {rel}")
        target.parent.mkdir(parents=True, exist_ok=True)
        with archive.open(info) as src, open(target, "wb") as dst:
            dst.write(src.read(MAX_UNPACKED + 1))
        (c_files if rel.lower().endswith(".c") else h_files).append(rel)
    deviations = ""
    if deviation_info is not None and deviation_info.file_size <= MAX_DEVIATION_BYTES:
        deviations = archive.read(deviation_info).decode("utf-8", "replace")
    project = Project(owner, name, tmp, root, c_files, h_files, deviations)
    State.projects[project.id] = project
    mine = [p for p in State.projects.values() if p.owner == owner]
    for old in mine[:-MAX_PROJECTS_PER_USER]:
        State.projects.pop(old.id, None)
    return project


def project_for(session: dict, project_id: str) -> Project:
    project = State.projects.get(project_id)
    if project is None or (project.owner != session["user"] and session["role"] != "admin"):
        raise ValueError("project not found; upload it again")
    return project


def analyze_project(session: dict, project_id: str, std: str, defines: str, includes: str,
                    deviations: str) -> dict:
    project = project_for(session, project_id)
    if std not in STANDARDS:
        raise ValueError("unsupported language standard")
    if not project.c_files:
        raise ValueError("the project has no .c files to analyze")
    macros = [d for d in re.split(r"[\s,]+", defines.strip()) if d]
    if any(not DEFINE_RE.match(d) for d in macros):
        raise ValueError("defines must look like NAME or NAME=value")
    extra = []
    for line in includes.splitlines():
        line = line.strip().strip("/")
        if line:
            resolved = (project.root / line).resolve()
            if project.root not in (resolved, *resolved.parents) or not resolved.is_dir():
                raise ValueError(f"include directory not in project: {line}")
            extra.append(str(resolved))
    flags = [f"-std={std}", *(f"-D{d}" for d in macros),
             *("-I" + str(project.root / d) if d else "-I" + str(project.root) for d in project.include_dirs),
             *("-I" + e for e in extra)]
    db = [{"directory": str(project.root), "file": str(project.root / rel),
           "arguments": [State.compiler, *flags, "-c", str(project.root / rel)]}
          for rel in sorted(project.c_files)]
    with tempfile.TemporaryDirectory(prefix="misra-db-") as t:
        db_path = Path(t) / "compile_commands.json"
        db_path.write_text(json.dumps(db))
        # A deviation file sent with the request wins over the one shipped in the zip.
        result = run_checker(db_path, timeout=900, deviations=deviations or project.deviations,
                             source_root=str(project.root))
    prefix = str(project.root) + "/"
    for f in result["findings"] + result["deviated"]:
        f["file"] = f["file"].replace(prefix, "")
        f["library"] = bool(LIBRARY_DIR.search(f["file"]))
    for key in ("compiler_output", "error"):
        result[key] = result[key].replace(prefix, "")[:20000]
    result["project"] = project.describe() | {"files": []}
    return result


def read_project_file(session: dict, project_id: str, rel: str) -> str:
    project = project_for(session, project_id)
    target = (project.root / rel).resolve()
    if project.root not in target.parents or target.suffix.lower() not in (".c", ".h"):
        raise ValueError("file not in project")
    return target.read_text(errors="replace")[:2_000_000]


class Handler(BaseHTTPRequestHandler):
    server_version = "misra-gui"
    timeout = 120  # drop stalled connections

    def log_message(self, fmt: str, *args) -> None:  # quiet
        pass

    # ---- helpers
    def _host_ok(self) -> bool:
        host = (self.headers.get("Host") or "").rsplit(":", 1)[0].strip("[]")
        return host in State.allowed_hosts

    def _origin_ok(self) -> bool:
        origin = self.headers.get("Origin")
        if not origin:
            return True
        return urlparse(origin).netloc == (self.headers.get("Host") or "")

    def _send(self, status: int, body: bytes, ctype: str, headers: dict | None = None) -> None:
        self.send_response(status)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("X-Frame-Options", "DENY")
        self.send_header("Referrer-Policy", "no-referrer")
        self.send_header("Content-Security-Policy",
                         "default-src 'self'; script-src 'self' 'unsafe-inline'; "
                         "style-src 'self' 'unsafe-inline'; img-src 'self' data:; "
                         "connect-src 'self'; frame-ancestors 'none'; base-uri 'none'; form-action 'self'")
        for key, value in (headers or {}).items():
            self.send_header(key, value)
        self.end_headers()
        self.wfile.write(body)

    def _json(self, status: int, payload, headers: dict | None = None) -> None:
        self._send(status, json.dumps(payload).encode(), "application/json", headers)

    def _client(self) -> str:
        return self.client_address[0]

    def _session(self) -> dict | None:
        if not State.require_auth:
            return {"user": "local", "role": "admin", "csrf": "", "token": ""}
        cookie = SimpleCookie(self.headers.get("Cookie") or "")
        morsel = cookie.get(COOKIE)
        return State.sessions.get(morsel.value if morsel else None)

    def _authorize(self, method: str, path: str) -> dict | None:
        """Return the session, or send the error response and return None."""
        session = self._session()
        if session is None:
            self._json(401, {"error": "login required"})
            return None
        need = ROUTE_ROLE.get((method, path), "admin")
        if auth.ROLES[session["role"]] < auth.ROLES[need]:
            self._json(403, {"error": f"the {need} role is required"})
            return None
        if method == "POST" and State.require_auth:
            if not secrets.compare_digest(self.headers.get("X-CSRF-Token") or "", session["csrf"]):
                self._json(403, {"error": "bad CSRF token; reload the page"})
                return None
        return session

    def _read_body(self, limit: int) -> bytes | None:
        length = int(self.headers.get("Content-Length") or 0)
        if length > limit:
            self._json(413, {"error": "request too large"})
            return None
        return self.rfile.read(length)

    # ---- GET
    def do_GET(self) -> None:  # noqa: N802
        if not self._host_ok():
            return self._json(403, {"error": "bad host"})
        url = urlparse(self.path)
        query = parse_qs(url.query)
        if url.path == "/":
            return self._send(200, INDEX.read_bytes(), "text/html; charset=utf-8")
        if url.path == "/api/me":
            session = self._session()
            if session is None:
                return self._json(401, {"error": "login required", "auth": State.require_auth})
            return self._json(200, {"user": session["user"], "role": session["role"],
                                    "csrf": session["csrf"], "auth": State.require_auth,
                                    "db_paths": bool(State.db_roots) and session["role"] == "admin"})
        session = self._authorize("GET", url.path)
        if session is None:
            return None
        try:
            if url.path == "/api/rules":
                return self._json(200, {"rules": load_rule_table(),
                                        "version": subprocess.run(
                                            [str(State.checker), "--version"],
                                            capture_output=True, text=True).stdout.strip()})
            if url.path == "/api/validate":
                rule = (query.get("rule") or [""])[0]
                if not re.fullmatch(r"\d+\.\d+", rule):
                    return self._json(400, {"error": "bad rule id"})
                return self._json(200, validate_one(rule, with_source=True))
            if url.path == "/api/validate-all":
                results = [validate_one(r, with_source=False)
                           for r in sorted(vr.list_rules(State.checker), key=vr.rule_sort_key)]
                return self._json(200, {"results": results})
            if url.path == "/api/file":
                return self._json(200, {"source": read_project_file(
                    session, (query.get("project") or [""])[0], (query.get("path") or [""])[0])})
            if url.path == "/api/samples":
                samples = [{"name": name, "rule": rule, "kind": kind,
                            "code": srcs[0].read_text(errors="replace")}
                           for rule, entries in sorted(State.cases.items(), key=lambda kv: vr.rule_sort_key(kv[0]))
                           for name, kind, srcs in entries if kind == "bad" and len(srcs) == 1]
                return self._json(200, {"samples": samples})
        except Exception as exc:  # report to the UI rather than dropping the connection
            return self._json(400, {"error": str(exc)})
        return self._json(404, {"error": "not found"})

    # ---- POST
    def do_POST(self) -> None:  # noqa: N802
        if not self._host_ok() or not self._origin_ok():
            return self._json(403, {"error": "bad host or origin"})
        url = urlparse(self.path)
        if url.path == "/api/login":
            return self._login()
        if url.path == "/api/logout":
            session = self._session()
            if session and State.require_auth:
                State.sessions.delete(session["token"])
                State.audit.write(session["user"], "logout", client=self._client())
            return self._json(200, {"ok": True}, {"Set-Cookie": self._cookie("", 0)})
        if (("POST", url.path)) not in ROUTE_ROLE:
            return self._json(404, {"error": "not found"})
        session = self._authorize("POST", url.path)
        if session is None:
            return None
        raw = self._read_body(MAX_ZIP if url.path == "/api/project" else MAX_BODY)
        if raw is None:
            return None
        try:
            if url.path == "/api/project":
                name = (parse_qs(url.query).get("name") or ["project.zip"])[0][:100]
                project = create_project(session["user"], name, raw)
                State.audit.write(session["user"], "project_upload", name=name,
                                  c_files=len(project.c_files), h_files=len(project.h_files))
                return self._json(200, project.describe())
            body = json.loads(raw or b"{}")
            deviations = str(body.get("deviations", ""))
            if body.get("project"):
                result = analyze_project(session, str(body["project"]), str(body.get("std", "c99")),
                                         str(body.get("defines", "")), str(body.get("includes", "")),
                                         deviations)
                mode = "project"
            elif body.get("compile_commands"):
                if session["role"] != "admin":
                    return self._json(403, {"error": "the admin role is required"})
                result = analyze_database(str(body["compile_commands"]), deviations)
                mode = "database"
            else:
                files = body.get("files") or [{"name": "input.c", "code": str(body.get("code", ""))}]
                result = analyze_files(files, str(body.get("std", "c99")), deviations)
                mode = "files"
            State.audit.write(session["user"], "analyze", mode=mode, findings=len(result["findings"]),
                              deviated=len(result["deviated"]), elapsed_ms=result["elapsed_ms"])
            return self._json(200, result)
        except subprocess.TimeoutExpired:
            return self._json(504, {"error": "analysis timed out"})
        except Exception as exc:
            return self._json(400, {"error": str(exc)})

    def _cookie(self, value: str, max_age: int) -> str:
        flags = "; Secure" if State.secure_cookie else ""
        return f"{COOKIE}={value}; Path=/; HttpOnly; SameSite=Strict; Max-Age={max_age}{flags}"

    def _login(self) -> None:
        raw = self._read_body(4096)
        if raw is None:
            return None
        try:
            body = json.loads(raw or b"{}")
            name, password = str(body.get("username", "")), str(body.get("password", ""))
        except ValueError:
            return self._json(400, {"error": "bad request"})
        wait = State.limiter.locked_for(self._client(), name)
        if wait:
            State.audit.write(name, "login_locked", client=self._client())
            return self._json(429, {"error": f"too many failed attempts; try again in {wait} s"})
        role = State.users.verify(name, password)
        if role is None:
            State.limiter.failed(self._client(), name)
            State.audit.write(name, "login_failed", client=self._client())
            return self._json(401, {"error": "invalid user name or password"})
        State.limiter.succeeded(self._client(), name)
        session = State.sessions.create(name, role)
        State.audit.write(name, "login", client=self._client(), role=role)
        return self._json(200, {"user": name, "role": role, "csrf": session["csrf"]},
                          {"Set-Cookie": self._cookie(session["token"], auth.SESSION_MAX_SECONDS)})


def bootstrap_admin(data_dir: Path) -> None:
    """First start: make sure an administrator exists."""
    if not State.users.is_empty():
        return
    password = os.environ.get("MISRA_GUI_ADMIN_PASSWORD")
    generated = not password
    password = password or secrets.token_urlsafe(15)
    State.users.add("admin", password, "admin")
    State.audit.write("system", "bootstrap_admin")
    if generated:
        print("=" * 60)
        print(f"First start: created user 'admin' with password: {password}")
        print(f"(stored hashed in {data_dir / 'users.json'}; change it with --add-user admin --role admin)")
        print("=" * 60, flush=True)
    else:
        print("First start: created user 'admin' from MISRA_GUI_ADMIN_PASSWORD", flush=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--host", default="127.0.0.1",
                        help="bind address (0.0.0.0 only inside a container whose port is published to loopback, or behind TLS)")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--checker", type=Path, default=None)
    parser.add_argument("--no-browser", action="store_true")
    parser.add_argument("--data-dir", type=Path,
                        default=Path(os.environ.get("MISRA_GUI_DATA", Path.home() / ".misra-gui")),
                        help="users file and audit log (env MISRA_GUI_DATA)")
    parser.add_argument("--no-auth", action="store_true", help="disable login (loopback only)")
    parser.add_argument("--allowed-host", action="append", default=[],
                        help="extra Host header value to accept (needed when not used via localhost)")
    parser.add_argument("--db-root", action="append", type=Path, default=[],
                        help="directory under which admins may analyze server-side compile databases")
    parser.add_argument("--tls-cert", type=Path)
    parser.add_argument("--tls-key", type=Path)
    parser.add_argument("--add-user", metavar="NAME")
    parser.add_argument("--role", choices=list(auth.ROLES), default="analyst")
    parser.add_argument("--remove-user", metavar="NAME")
    parser.add_argument("--list-users", action="store_true")
    args = parser.parse_args()

    State.users = auth.UserStore(args.data_dir / "users.json")
    State.audit = auth.AuditLog(args.data_dir / "audit.log")
    if args.add_user or args.remove_user or args.list_users:
        try:
            if args.add_user:
                State.users.add(args.add_user, auth.read_password(), args.role)
                State.audit.write("cli", "user_add", name=args.add_user, role=args.role)
                print(f"user '{args.add_user}' saved with role {args.role}")
            if args.remove_user:
                removed = State.users.remove(args.remove_user)
                State.audit.write("cli", "user_remove", name=args.remove_user, removed=removed)
                print("removed" if removed else "no such user")
            if args.list_users:
                for name, role in sorted(State.users.names().items()):
                    print(f"{name}\t{role}")
        except ValueError as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 2
        return 0

    loopback = args.host in ("127.0.0.1", "localhost", "::1")
    if args.no_auth and not loopback:
        print("error: --no-auth is only allowed when bound to a loopback address", file=sys.stderr)
        return 2
    State.require_auth = not args.no_auth
    State.allowed_hosts |= set(args.allowed_host)
    State.db_roots = [p.resolve() for p in args.db_root]
    if args.no_auth:
        State.db_roots = State.db_roots or [Path.home().resolve()]
        print("WARNING: login is disabled; anyone who can reach this port has full access.")
    else:
        bootstrap_admin(args.data_dir)

    State.checker = args.checker or vr.find_checker()
    State.compiler = vr.find_clang()
    State.cases = vr.discover_cases()

    server = ThreadingHTTPServer((args.host, args.port), Handler)
    scheme = "http"
    if args.tls_cert and args.tls_key:
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.minimum_version = ssl.TLSVersion.TLSv1_2
        context.load_cert_chain(args.tls_cert, args.tls_key)
        server.socket = context.wrap_socket(server.socket, server_side=True)
        State.secure_cookie, scheme = True, "https"
    url = f"{scheme}://127.0.0.1:{args.port}/"
    print(f"misra-gui serving {url} (checker: {State.checker}, login {'off' if args.no_auth else 'on'})", flush=True)
    if not args.no_browser:
        webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
