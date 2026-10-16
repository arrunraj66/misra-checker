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


FAKE_GCC = '''#!/bin/sh
case "$*" in
  *--version*) echo "arm-none-eabi-gcc (Fake Toolchain) 10.3.1"; echo "Copyright Free Software Foundation";;
  *-dumpmachine*) echo arm-none-eabi;;
  *-dM*) printf '#define __GNUC__ 10\\n#define __GNUC_MINOR__ 3\\n#define __GNUC_PATCHLEVEL__ 1\\n';;
  *"-E -x c -v"*) cat >&2 <<EOF
#include <...> search starts here:
 INCLUDE_DIR
End of search list.
EOF
  ;;
esac
'''

with tempfile.TemporaryDirectory() as data_dir:
    fake_dir = Path(data_dir) / "fake-toolchain"
    (fake_dir / "include").mkdir(parents=True)
    (fake_dir / "include" / "fakelib.h").write_text("#define FAKE_NEWLIB 1\n")
    fake_cc = fake_dir / "arm-none-eabi-gcc"
    fake_cc.write_text(FAKE_GCC.replace("INCLUDE_DIR", str(fake_dir / "include")))
    fake_cc.chmod(0o755)
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
                               "--no-browser", "--checker", CHECKER, "--db-root", outside,
                               "--compiler", f"arm-fake={fake_cc}",
                               "--promote-dir", str(Path(data_dir) / "promoted")],
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

        # ---- project compilers
        status, listing, _ = bob.call("GET", "/api/compilers")
        ids = {c["id"]: c for c in listing["compilers"]} if status == 200 else {}
        check("compilers listed", status == 200 and "default" in ids and ids["arm-fake"]["usable"], listing)
        check("anonymous cannot list compilers", Client(port).call("GET", "/api/compilers")[0] == 401)
        arm_code = '#include <fakelib.h>\n_Static_assert(sizeof(long) == 4, "32-bit long");\nint v = FAKE_NEWLIB;\n'
        status, res, _ = bob.call("POST", "/api/analyze", {"files": [{"name": "a.c", "code": arm_code}],
                                                           "compiler": "arm-fake", "flags": "-mcpu=cortex-m4 -mthumb -DSTM32F4"})
        check("ARM profile compiles the project", status == 200 and not res["compile_errors"], res)
        check("report names the compiler", "arm-none-eabi" in res.get("compiler", ""), res.get("compiler"))
        status, res, _ = bob.call("POST", "/api/analyze", {"files": [{"name": "a.c", "code": arm_code}]})
        check("default profile does not", status == 200 and res["compile_errors"], res)
        status, res, _ = bob.call("POST", "/api/analyze", {"files": [{"name": "a.c", "code": "int x;"}], "compiler": "nonexistent"})
        check("unknown compiler rejected", status == 400 and "unknown compiler" in res["error"], res)
        status, res, _ = bob.call("POST", "/api/analyze", {"files": [{"name": "a.c", "code": "int x;"}],
                                                           "flags": "-include /etc/passwd"})
        check("dangerous flag rejected", status == 400 and "not allowed" in res["error"], res)
        buf2 = io.BytesIO()
        with zipfile.ZipFile(buf2, "w") as z:
            z.writestr("src/m.c", arm_code)
        status, proj2, _ = bob.call("POST", "/api/project?name=arm.zip", buf2.getvalue(), raw=True,
                                    headers={"Content-Type": "application/octet-stream"})
        status, res, _ = bob.call("POST", "/api/analyze", {"project": proj2["id"], "compiler": "arm-fake", "std": "gnu99"})
        check("project honours the compiler and gnu99", status == 200 and not res["compile_errors"], res)

        # ---- a missing header is named, and does not cause a bogus Rule 17.4
        miss_code = ('#include "no_such_header.h"\nstatic volatile uint32_t g_ms = 0U;\n'
                     'static uint32_t millis(void)\n{\n    return g_ms;\n}\n')
        status, res, _ = bob.call("POST", "/api/analyze", {"files": [{"name": "m.c", "code": miss_code}]})
        check("missing header reported", status == 200 and res["missing_headers"] == ["no_such_header.h"], res.get("missing_headers"))
        check("no Rule 17.4 after a missing header", not any(f["rule"] == "17.4" for f in res["findings"]), res["findings"])

        # ---- editing a project inside the GUI
        def put(client, project_id, path, text):
            return client.call("POST", f"/api/file?project={project_id}&path={path}", text.encode(), raw=True,
                               headers={"Content-Type": "text/plain"})
        status, saved, _ = put(bob, proj["id"], "src/m.c", "int edited = 1;\n")
        check("owner can save a file", status == 200 and saved["saved"] and not saved["new"], saved)
        status, content, _ = bob.call("GET", f"/api/file?project={proj['id']}&path=src/m.c")
        check("saved text is read back", content.get("source") == "int edited = 1;\n", content)
        status, saved, _ = put(bob, proj["id"], "inc/new.h", "#define NEW 1\n")
        check("new file is created and listed", status == 200 and saved["new"]
              and "inc/new.h" in [f["path"] for f in saved["project"]["files"]], saved)
        for bad_path in ("../escape.c", "/etc/cron.d/x.c", "a/../../b.c", "notes.txt", "src/m.c%00.txt"):
            status, res, _ = put(bob, proj["id"], bad_path, "x")
            check(f"unsafe path rejected: {bad_path}", status == 400, (status, res))
        check("other analyst cannot save into a foreign project", put(erin, proj["id"], "src/m.c", "x")[0] == 400)
        check("viewer cannot save", put(carol, proj["id"], "src/m.c", "x")[0] == 403)
        status, res, _ = bob.call("POST", "/api/analyze", {"project": proj["id"]})
        check("analysis sees the edited file", status == 200 and not any(f["rule"] == "7.1" for f in res["findings"]), res["findings"][:3])
        status, zdata, _ = bob.call("GET", f"/api/project-zip?project={proj['id']}")
        check("edited project downloads as a zip", status == 200 and isinstance(zdata, bytes) and zdata[:2] == b"PK")
        with zipfile.ZipFile(io.BytesIO(zdata)) as z:
            check("zip holds the edits", z.read("src/m.c") == b"int edited = 1;\n" and "inc/new.h" in z.namelist(), z.namelist())
        check("zip is private to the owner", erin.call("GET", f"/api/project-zip?project={proj['id']}")[0] == 400)

        # ---- triage: false positives, missed violations and replay
        tcode = "int f(void)\n{\n    int x = 010;\n    return x;\n}\n"
        def triage(client, **body):
            return client.call("POST", "/api/triage", {"files": [{"name": "t.c", "code": tcode}], "std": "c99", **body})
        status, made, _ = triage(bob, kind="true-positive", rule="7.1", file="t.c", line=3, note="octal")
        check("true positive saved", status == 200 and made["case"]["kind"] == "true-positive" and "files" not in made["case"], made)
        status, fp, _ = triage(bob, kind="false-positive", rule="7.1", file="t.c", line=3, note="really a decimal value")
        fp_id = fp["case"]["id"]
        status, fn, _ = triage(bob, kind="false-negative", rule="15.5", file="t.c", line=4, note="early return")
        status, again, _ = triage(bob, kind="false-positive", rule="7.1", file="t.c", line=3, note="updated note")
        check("same verdict is updated, not duplicated", again["case"]["id"] == fp_id, again)
        status, listing, _ = bob.call("GET", "/api/triage")
        check("cases are listed with per-rule totals", status == 200 and len(listing["cases"]) == 3
              and listing["summary"]["7.1"]["false_positive"] == 1 and listing["summary"]["7.1"]["true_positive"] == 1
              and listing["summary"]["15.5"]["false_negative"] == 1, listing)
        check("precision follows the verdicts", listing["summary"]["7.1"]["precision"] == 0.5, listing["summary"])
        for bad in (dict(kind="maybe", rule="7.1", file="t.c", line=3), dict(kind="false-positive", rule="x", file="t.c", line=3),
                    dict(kind="false-positive", rule="7.1", file="other.c", line=3),
                    dict(kind="false-positive", rule="7.1", file="t.c", line=99)):
            status, res, _ = triage(bob, **bad)
            check(f"bad triage request rejected: {bad}", status == 400, (status, res))
        check("viewer cannot triage", triage(carol, kind="true-positive", rule="7.1", file="t.c", line=3)[0] == 403)
        status, theirs, _ = erin.call("GET", "/api/triage")
        check("cases are private to their author", status == 200 and theirs["cases"] == [], theirs)
        status, replay, _ = bob.call("POST", "/api/triage/run", {})
        by_id = {r["id"]: r for r in replay["results"]}
        check("replay: the false positive is still reported (open)", by_id[fp_id]["state"] == "open", replay)
        check("replay: the true positive holds (fixed)", any(r["state"] == "fixed" for r in replay["results"]), replay)
        status, listing, _ = bob.call("GET", "/api/triage")
        check("replay stores the state", {c["id"]: c["state"] for c in listing["cases"]}[fp_id] == "open", listing)
        status, exp, _ = bob.call("GET", "/api/triage-export")
        check("cases export as a zip", status == 200 and isinstance(exp, bytes) and exp[:2] == b"PK")
        check("analyst cannot promote", bob.call("POST", "/api/triage/promote", {})[0] == 403)
        status, promo, _ = alice.call("POST", "/api/triage/promote", {})
        promoted_files = sorted(p.name for p in (Path(data_dir) / "promoted").glob("*.json")) if status == 200 else []
        check("admin promotes cases into a test directory", status == 200 and len(promoted_files) >= 3
              and all(n[:3] in ("fp-", "fn-", "tp-") for n in promoted_files), (status, promo, promoted_files))
        promoted = json.loads((Path(data_dir) / "promoted" / promoted_files[0]).read_text())
        check("promoted cases carry no user name", "user" not in promoted and promoted["files"], list(promoted))
        status, gone, _ = bob.call("POST", "/api/triage/delete", {"id": fp_id})
        check("author can delete a case", status == 200 and gone["deleted"] is True, gone)
        check("a deleted case is gone", fp_id not in [c["id"] for c in bob.call("GET", "/api/triage")[1]["cases"]])

        # ---- cppcheck cross-verification
        status, me2, _ = bob.call("GET", "/api/me")
        has_cppcheck = me2["tools"]["cppcheck"]["available"]
        xcode = "int g(void)\n{\n    return 1;\n}\nint f(int a)\n{\n    goto end;\nend:\n    return a;\n}\n"
        status, res, _ = bob.call("POST", "/api/analyze", {"files": [{"name": "x.c", "code": xcode}], "cppcheck": True})
        if has_cppcheck:
            cross = res.get("cross", {})
            check("cppcheck cross-check runs", status == 200 and cross.get("agree", 0) >= 2, res.get("error") or cross)
            check("findings are annotated", any(f.get("cross") == "both" for f in res["findings"]), res["findings"][:3])
            check("cppcheck version reported", "Cppcheck" in cross.get("tool", ""), cross.get("tool"))
            status, res, _ = bob.call("POST", "/api/analyze", {"files": [{"name": "x.c", "code": xcode}]})
            check("cross-check is opt-in", "cross" not in res)
        else:
            check("cross-check refused without cppcheck", status == 400 and "cppcheck" in res["error"], res)

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
