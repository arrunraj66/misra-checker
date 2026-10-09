#!/usr/bin/env python3
"""Local web GUI for misra-checker.

Serves a single page on 127.0.0.1 that can

* analyze pasted/uploaded C code or an existing compile_commands.json,
* list all rules with their per-rule fixture validation status,
* validate rules one at a time (or all) and show the bad/ok fixtures.

Only the Python standard library is used. The server binds to loopback only and
rejects requests whose Host header is not loopback (DNS-rebinding guard).

    python3 gui/misra_gui.py [--port 8765] [--checker build/misra-checker]
"""

from __future__ import annotations

import argparse
import io
import json
import re
import subprocess
import sys
import tempfile
import time
import uuid
import webbrowser
import zipfile
from collections import OrderedDict
from dataclasses import asdict
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))
import validate_rules as vr  # noqa: E402

INDEX = Path(__file__).resolve().parent / "index.html"
MAX_BODY = 8 * 1024 * 1024
MAX_ZIP = 100 * 1024 * 1024
MAX_UNPACKED = 300 * 1024 * 1024
MAX_PROJECT_FILES = 10000
MAX_PROJECTS = 3
LIBRARY_DIR = re.compile(
    r"(^|/)(libs?|librar(y|ies)|third[_-]?party|vendors?|externa?l?|drivers?|hal|cmsis|middlewares?|sdk|thirdparty)(/|$)",
    re.I)
DEFINE_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*(=[^\s]*)?$")
FINDING_RE = re.compile(
    r"^(?P<file>.+?):(?P<line>\d+):(?P<col>\d+): (?P<category>\w+): "
    r"MISRA C:2012 Rule (?P<rule>\S+) \[(?P<key>[^\]]+)\]$")
STANDARDS = {"c90", "c99", "c11", "c17"}


class State:
    projects: "OrderedDict[str, Project]" = OrderedDict()
    checker: Path
    compiler: str
    cases: dict


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


def parse_findings(stdout: str) -> tuple[list[dict], str]:
    findings, summary = [], []
    for line in stdout.splitlines():
        m = FINDING_RE.match(line)
        if m:
            d = m.groupdict()
            d["line"], d["col"] = int(d["line"]), int(d["col"])
            d["message"] = re.sub(r"^misra-c2012-\d+\.\d+-", "", d["key"]).replace("-", " ")
            if d not in findings:
                findings.append(d)
        elif line.strip():
            summary.append(line.strip())
    return findings, " ".join(summary)


def run_checker(db_path: Path, timeout: int = 300) -> dict:
    started = time.monotonic()
    proc = subprocess.run(
        [str(State.checker), "analyze", "--compile-commands", str(db_path)],
        capture_output=True, text=True, timeout=timeout)
    elapsed_ms = round((time.monotonic() - started) * 1000)
    findings, summary = parse_findings(proc.stdout)
    compile_errors = "results may be incomplete" in proc.stderr
    return {"exit_code": proc.returncode, "findings": findings, "summary": summary,
            "elapsed_ms": elapsed_ms, "compile_errors": compile_errors,
            "compiler_output": proc.stderr.strip() if compile_errors else "",
            "error": proc.stderr.strip() if proc.returncode not in (0, 1) and not compile_errors else ""}


SAFE_NAME = re.compile(r"^[A-Za-z0-9_][A-Za-z0-9_.-]{0,63}\.[ch]$")


def analyze_files(files: list[dict], std: str) -> dict:
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
        result = run_checker(db)
        prefix = tmp + "/"
        result["compiler_output"] = result["compiler_output"].replace(prefix, "")
        result["error"] = result["error"].replace(prefix, "")
    for f in result["findings"]:
        f["file"] = f["file"].replace(prefix, "")
    return result


def analyze_database(path: str) -> dict:
    p = Path(path).expanduser()
    if p.is_dir():
        p = p / "compile_commands.json"
    if not p.is_file():
        raise ValueError(f"no such compile database: {p}")
    return run_checker(p)


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
    """A zip uploaded by the user, unpacked (C sources and headers only) in a temp dir."""

    def __init__(self, name: str, tmp: tempfile.TemporaryDirectory, root: Path,
                 c_files: list[str], h_files: list[str]) -> None:
        self.id = uuid.uuid4().hex[:12]
        self.name, self._tmp, self.root = name, tmp, root
        self.c_files, self.h_files = c_files, h_files

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
                "files": [{"path": f, "kind": "c" if f.endswith(".c") else "h",
                           "library": bool(LIBRARY_DIR.search(f))}
                          for f in sorted(self.c_files + self.h_files)]}


def create_project(name: str, data: bytes) -> Project:
    try:
        archive = zipfile.ZipFile(io.BytesIO(data))
    except zipfile.BadZipFile as exc:
        raise ValueError("not a valid zip archive") from exc
    wanted = []
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
    project = Project(name, tmp, root, c_files, h_files)
    State.projects[project.id] = project
    while len(State.projects) > MAX_PROJECTS:
        State.projects.popitem(last=False)
    return project


def analyze_project(project_id: str, std: str, defines: str, includes: str) -> dict:
    project = State.projects.get(project_id)
    if project is None:
        raise ValueError("project not found; upload it again")
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
        result = run_checker(db_path, timeout=900)
    prefix = str(project.root) + "/"
    for f in result["findings"]:
        f["file"] = f["file"].replace(prefix, "")
        f["library"] = bool(LIBRARY_DIR.search(f["file"]))
    for key in ("compiler_output", "error"):
        result[key] = result[key].replace(prefix, "")[:20000]
    result["project"] = project.describe() | {"files": []}
    return result


def read_project_file(project_id: str, rel: str) -> str:
    project = State.projects.get(project_id)
    if project is None:
        raise ValueError("project not found")
    target = (project.root / rel).resolve()
    if project.root not in target.parents or target.suffix.lower() not in (".c", ".h"):
        raise ValueError("file not in project")
    return target.read_text(errors="replace")[:2_000_000]


class Handler(BaseHTTPRequestHandler):
    server_version = "misra-gui"

    def log_message(self, fmt: str, *args) -> None:  # quiet
        pass

    def _host_ok(self) -> bool:
        host = (self.headers.get("Host") or "").rsplit(":", 1)[0].strip("[]")
        return host in ("127.0.0.1", "localhost", "::1")

    def _send(self, status: int, body: bytes, ctype: str) -> None:
        self.send_response(status)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        self.wfile.write(body)

    def _json(self, status: int, payload) -> None:
        self._send(status, json.dumps(payload).encode(), "application/json")

    def do_GET(self) -> None:  # noqa: N802
        if not self._host_ok():
            return self._json(403, {"error": "bad host"})
        url = urlparse(self.path)
        query = parse_qs(url.query)
        try:
            if url.path == "/":
                return self._send(200, INDEX.read_bytes(), "text/html; charset=utf-8")
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
                    (query.get("project") or [""])[0], (query.get("path") or [""])[0])})
            if url.path == "/api/samples":
                samples = [{"name": name, "rule": rule, "kind": kind,
                            "code": srcs[0].read_text(errors="replace")}
                           for rule, entries in sorted(State.cases.items(), key=lambda kv: vr.rule_sort_key(kv[0]))
                           for name, kind, srcs in entries if kind == "bad" and len(srcs) == 1]
                return self._json(200, {"samples": samples})
        except Exception as exc:  # report to the UI rather than dropping the connection
            return self._json(500, {"error": str(exc)})
        self._json(404, {"error": "not found"})

    def do_POST(self) -> None:  # noqa: N802
        if not self._host_ok():
            return self._json(403, {"error": "bad host"})
        url = urlparse(self.path)
        if url.path not in ("/api/analyze", "/api/project"):
            return self._json(404, {"error": "not found"})
        length = int(self.headers.get("Content-Length") or 0)
        if length > (MAX_ZIP if url.path == "/api/project" else MAX_BODY):
            return self._json(413, {"error": "request too large"})
        try:
            raw = self.rfile.read(length)
            if url.path == "/api/project":
                name = (parse_qs(url.query).get("name") or ["project.zip"])[0][:100]
                return self._json(200, create_project(name, raw).describe())
            body = json.loads(raw or b"{}")
            if body.get("project"):
                result = analyze_project(str(body["project"]), str(body.get("std", "c99")),
                                         str(body.get("defines", "")), str(body.get("includes", "")))
            elif body.get("compile_commands"):
                result = analyze_database(str(body["compile_commands"]))
            else:
                files = body.get("files") or [{"name": "input.c", "code": str(body.get("code", ""))}]
                result = analyze_files(files, str(body.get("std", "c99")))
            return self._json(200, result)
        except subprocess.TimeoutExpired:
            return self._json(504, {"error": "analysis timed out"})
        except Exception as exc:
            return self._json(400, {"error": str(exc)})


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--host", default="127.0.0.1",
                        help="bind address (0.0.0.0 only inside a container whose port is published to loopback)")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--checker", type=Path, default=None)
    parser.add_argument("--no-browser", action="store_true")
    args = parser.parse_args()

    State.checker = args.checker or vr.find_checker()
    State.compiler = vr.find_clang()
    State.cases = vr.discover_cases()

    server = ThreadingHTTPServer((args.host, args.port), Handler)
    url = f"http://127.0.0.1:{args.port}/"
    print(f"misra-gui serving {url} (checker: {State.checker})")
    if not args.no_browser:
        webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
