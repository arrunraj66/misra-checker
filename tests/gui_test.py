#!/usr/bin/env python3
"""GUI server: login, roles, lockout, CSRF, project ownership, SARIF, deviations."""
import http.client
import io
import json
import os
import socket
import subprocess
import sys
import tempfile
import time
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SERVER = ROOT / "gui" / "misra_gui.py"
CHECKER = sys.argv[1]
failures = []


def check(name, condition, detail=""):
    if not condition:
        failures.append(f"{name}: {detail}")


def free_port():
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def cli(data_dir, *args, password=None):
    env = {**os.environ, "MISRA_GUI_PASSWORD": password or ""}
    return subprocess.run([sys.executable, str(SERVER), "--data-dir", data_dir, *args],
                          capture_output=True, text=True, env=env)


class Client:
    def __init__(self, port):
        self.port, self.cookie, self.csrf = port, None, ""

    def call(self, method, path, body=None, headers=None, raw=False):
        conn = http.client.HTTPConnection("127.0.0.1", self.port, timeout=120)
        h = {"Host": f"127.0.0.1:{self.port}", **(headers or {})}
        if self.cookie:
            h["Cookie"] = self.cookie
        if self.csrf and method == "POST":
            h.setdefault("X-CSRF-Token", self.csrf)
        data = body if raw else (json.dumps(body).encode() if body is not None else None)
        conn.request(method, path, body=data, headers=h)
        r = conn.getresponse()
        text = r.read()
        set_cookie = r.getheader("Set-Cookie")
        conn.close()
        try:
            return r.status, json.loads(text), set_cookie
        except ValueError:
            return r.status, text, set_cookie

    def login(self, user, password):
        status, data, cookie = self.call("POST", "/api/login", {"username": user, "password": password})
        if status == 200:
            self.cookie = cookie.split(";")[0]
            self.csrf = data["csrf"]
        return status, data, cookie


with tempfile.TemporaryDirectory() as data_dir:
    # ---- user management CLI
    short = cli(data_dir, "--add-user", "bob", "--role", "analyst", password="short")
    check("short password rejected", short.returncode == 2 and "at least" in short.stderr, short.stderr)
    for name, role in (("alice", "admin"), ("bob", "analyst"), ("carol", "viewer"), ("dave", "analyst")):
        done = cli(data_dir, "--add-user", name, "--role", role, password="correct horse battery")
        check(f"add {name}", done.returncode == 0, done.stderr)
    listed = cli(data_dir, "--list-users").stdout
    check("list users", "alice\tadmin" in listed and "carol\tviewer" in listed, listed)
    check("hash stored, not password", "correct horse" not in (Path(data_dir) / "users.json").read_text())
    check("users file mode", oct((Path(data_dir) / "users.json").stat().st_mode & 0o777) == "0o600")
    refuse = subprocess.run([sys.executable, str(SERVER), "--data-dir", data_dir, "--no-auth", "--host", "0.0.0.0"],
                            capture_output=True, text=True)
    check("--no-auth refused off loopback", refuse.returncode == 2)

    port = free_port()
    outside = tempfile.mkdtemp()
    server = subprocess.Popen([sys.executable, str(SERVER), "--data-dir", data_dir, "--port", str(port),
                               "--no-browser", "--checker", CHECKER, "--db-root", outside],
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    try:
        for _ in range(100):
            try:
                socket.create_connection(("127.0.0.1", port), timeout=0.2).close()
                break
            except OSError:
                time.sleep(0.1)

        anon = Client(port)
        check("page is public", anon.call("GET", "/")[0] == 200)
        for path in ("/api/rules", "/api/samples", "/api/validate?rule=7.1", "/api/file?project=x&path=a.c"):
            check(f"anonymous {path} -> 401", anon.call("GET", path)[0] == 401)
        check("anonymous analyze -> 401", anon.call("POST", "/api/analyze", {"code": "int x;"})[0] == 401)
        check("/api/me anonymous", anon.call("GET", "/api/me")[0] == 401)

        # ---- login failures and lockout
        bad = Client(port)
        status, data, _ = bad.login("alice", "wrong password!!")
        check("wrong password", status == 401 and data["error"] == "invalid user name or password", data)
        status, data, _ = bad.login("nobody", "whatever whatever")
        check("unknown user same message", status == 401 and data["error"] == "invalid user name or password", data)
        for _ in range(5):
            bad.login("dave", "wrong password!!")
        check("locked after repeated failures", bad.login("dave", "correct horse battery")[0] == 429)
        check("lockout is per user", Client(port).login("bob", "correct horse battery")[0] == 200)

        # ---- cookie flags, CSRF, host and origin
        alice = Client(port)
        status, data, cookie = alice.login("alice", "correct horse battery")
        check("login ok", status == 200 and data["role"] == "admin", data)
        check("cookie flags", all(f in cookie for f in ("HttpOnly", "SameSite=Strict", "Path=/")), cookie)
        status, me, _ = alice.call("GET", "/api/me")
        check("me", status == 200 and me["user"] == "alice")
        no_csrf = alice.call("POST", "/api/analyze", {"code": "int x;"}, headers={"X-CSRF-Token": "wrong"})
        check("wrong CSRF -> 403", no_csrf[0] == 403, no_csrf)
        check("bad Host -> 403", alice.call("GET", "/api/rules", headers={"Host": "evil.com"})[0] == 403)
        cross = alice.call("POST", "/api/analyze", {"code": "int x;"}, headers={"Origin": "http://evil.com"})
        check("cross-origin POST -> 403", cross[0] == 403, cross)

        # ---- roles
        carol = Client(port)
        carol.login("carol", "correct horse battery")
        check("viewer can read rules", carol.call("GET", "/api/rules")[0] == 200)
        check("viewer cannot analyze", carol.call("POST", "/api/analyze", {"code": "int x;"})[0] == 403)
        check("viewer cannot validate", carol.call("GET", "/api/validate?rule=7.1")[0] == 403)
        bob = Client(port)
        bob.login("bob", "correct horse battery")
        check("analyst validates", bob.call("GET", "/api/validate?rule=7.1")[0] == 200)
        check("analyst cannot read server paths",
              bob.call("POST", "/api/analyze", {"compile_commands": outside})[0] == 403)
        status, data, _ = alice.call("POST", "/api/analyze", {"compile_commands": "/etc"})
        check("admin limited to --db-root", status == 400 and "outside" in data["error"], data)

        # ---- analysis, SARIF and deviations over the API
        code = "int v = 010;\n"
        status, res, _ = bob.call("POST", "/api/analyze", {"files": [{"name": "a.c", "code": code}], "std": "c99"})
        check("analyze ok", status == 200 and any(f["rule"] == "7.1" for f in res["findings"]), res)
        sarif = json.loads(res["sarif"])
        check("sarif returned", sarif["version"] == "2.1.0" and "originalUriBaseIds" not in sarif["runs"][0])
        dev = {"deviations": [{"id": "D1", "rules": ["7.1"], "file": "a.c", "justification": "j", "approved_by": "x"}]}
        status, res, _ = bob.call("POST", "/api/analyze",
                                  {"files": [{"name": "a.c", "code": code}], "deviations": json.dumps(dev)})
        check("deviation applied", status == 200 and not any(f["rule"] == "7.1" for f in res["findings"])
              and res["deviated"] and res["deviated"][0]["deviation"] == "D1", res)
        bad_dev = {"deviations": [{"id": "D2", "rules": ["9.1"], "justification": "j", "approved_by": "x"}]}
        status, res, _ = bob.call("POST", "/api/analyze",
                                  {"files": [{"name": "a.c", "code": code}], "deviations": json.dumps(bad_dev)})
        check("mandatory deviation rejected", "mandatory" in (res.get("error", "") if isinstance(res, dict) else ""), res)

        # ---- projects: ownership and bundled deviation file
        buf = io.BytesIO()
        with zipfile.ZipFile(buf, "w") as z:
            z.writestr("p/src/m.c", code)
            z.writestr("p/misra-deviations.json", json.dumps(dev).replace('"a.c"', '"src/m.c"'))
        status, proj, _ = bob.call("POST", "/api/project?name=p.zip", buf.getvalue(), raw=True,
                                   headers={"Content-Type": "application/octet-stream"})
        check("project upload", status == 200 and proj["has_deviations"], proj)
        status, res, _ = bob.call("POST", "/api/analyze", {"project": proj["id"]})
        check("project deviations from zip", status == 200 and res["deviated"], res)
        other = Client(port)
        # dave is locked out above; use alice (admin) and a fresh analyst for the ownership check
        cli(data_dir, "--add-user", "erin", "--role", "analyst", password="correct horse battery")
        erin = Client(port)
        erin.login("erin", "correct horse battery")
        check("other analyst cannot analyze foreign project",
              erin.call("POST", "/api/analyze", {"project": proj["id"]})[0] == 400)
        check("other analyst cannot read foreign file",
              erin.call("GET", f"/api/file?project={proj['id']}&path=src/m.c")[0] == 400)
        check("owner reads file", bob.call("GET", f"/api/file?project={proj['id']}&path=src/m.c")[0] == 200)
        check("admin reads any project",
              alice.call("GET", f"/api/file?project={proj['id']}&path=src/m.c")[0] == 200)

        # ---- logout invalidates the session
        token_cookie = bob.cookie
        check("logout", bob.call("POST", "/api/logout", {})[0] == 200)
        stale = Client(port)
        stale.cookie, stale.csrf = token_cookie, ""
        check("session dead after logout", stale.call("GET", "/api/rules")[0] == 401)

        audit = (Path(data_dir) / "audit.log").read_text()
        check("audit has login/failed/analyze",
              all(a in audit for a in ('"login"', '"login_failed"', '"login_locked"', '"analyze"', '"project_upload"')))
        check("audit never holds passwords", "correct horse" not in audit)
    finally:
        server.terminate()
        server.wait(timeout=10)

if failures:
    print("\n".join(failures))
    sys.exit(1)
print("gui tests passed")
