#!/usr/bin/env python3
"""End-to-end test of the GUI in a real browser (Chromium via Playwright).

Starts the server on a temporary data directory and drives the page the way a user does: login,
analysis, markers in the editor, triage, baselines, the Compare tab with cppcheck, a zip project with
editing, compliance report download, publishing to the shared history, theme and a narrow window.

Skipped (exit status 77) when Playwright or its browser is not installed:
    pip install playwright && python3 -m playwright install chromium

    python3 tests/e2e_browser.py [--screenshots DIR] [--headed]
"""
import argparse
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
try:
    from playwright.sync_api import sync_playwright, TimeoutError as PwTimeout
except ImportError:
    print("playwright is not installed; skipping")
    sys.exit(77)

failures = []


def check(name, condition, detail=""):
    if not condition:
        failures.append(f"{name}: {detail}")
        print(f"  FAIL {name}: {detail}")


CODE = """#include <stdint.h>
int a;
int compute(int x, int y)
{
    int r = x + y;
    if (r) { r++; }
    goto done;
done:
    return r;
}
int mask = 010;
"""
PASSWORD = "browser test pass 1"


def wait_until(page, expression, timeout=60.0):
    """Poll a JavaScript expression. page.wait_for_function evaluates strings, which the GUI's
    Content-Security-Policy (no unsafe-eval) correctly forbids."""
    end = time.time() + timeout
    while time.time() < end:
        if page.evaluate(expression):
            return
        time.sleep(0.1)
    raise AssertionError(f"timed out waiting for: {expression}")


def analyze(page, button="#run"):
    """Click Analyze and wait for the server's answer and for the page to finish rendering it."""
    with page.expect_response(lambda r: r.url.endswith("/api/analyze")):
        page.click(button)
    wait_until(page, "!document.getElementById('run').disabled && !document.querySelector('#result .spin')")


def free_port():
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--screenshots", type=Path)
    ap.add_argument("--headed", action="store_true")
    ap.add_argument("--checker", default=str(ROOT / "build" / "misra-checker"))
    args = ap.parse_args()
    shots = args.screenshots
    if shots:
        shots.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as data:
        env = {**os.environ, "MISRA_GUI_PASSWORD": PASSWORD}
        subprocess.run([sys.executable, str(ROOT / "gui" / "misra_gui.py"), "--data-dir", data, "--add-user", "tester", "--role", "admin"],
                       env=env, check=True, capture_output=True)
        port = free_port()
        server = subprocess.Popen([sys.executable, str(ROOT / "gui" / "misra_gui.py"), "--data-dir", data, "--port", str(port),
                                   "--no-browser", "--checker", args.checker, "--promote-dir", str(Path(data) / "promoted")],
                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        try:
            for _ in range(100):
                try:
                    socket.create_connection(("127.0.0.1", port), timeout=0.2).close()
                    break
                except OSError:
                    time.sleep(0.1)
            run(f"http://127.0.0.1:{port}", shots, args.headed, Path(data))
        finally:
            server.terminate()
            server.wait(timeout=10)

    if failures:
        print(f"{len(failures)} check(s) failed")
        return 1
    print("browser tests passed")
    return 0


def run(base, shots, headed, data):
    def shot(page, name):
        if shots:
            page.screenshot(path=str(shots / f"{name}.png"))

    with sync_playwright() as p:
        browser = p.chromium.launch(headless=not headed)
        context = browser.new_context(viewport={"width": 1440, "height": 900}, accept_downloads=True)
        page = context.new_page()
        errors = []
        page.on("pageerror", lambda e: errors.append(f"js: {e}"))
        page.on("console", lambda m: errors.append(f"console: {m.text}") if m.type == "error" and "401" not in m.text else None)
        page.set_default_timeout(60000)

        # ---- login
        page.goto(base + "/")
        page.wait_for_selector("#login:not([hidden])")
        check("login page is shown first", page.is_visible("#login-form"))
        check("the app is not visible before login", not page.is_visible("#view-analyze") or page.is_hidden("#userbox"))
        page.fill("#l-user", "tester"); page.fill("#l-pass", "wrong password!!"); page.click("#l-submit")
        page.wait_for_selector("#l-error:not([hidden])")
        check("a wrong password is refused with a message", "invalid" in page.inner_text("#l-error"))
        page.fill("#l-pass", PASSWORD); page.click("#l-submit")
        page.wait_for_selector("#login", state="hidden")
        check("the signed in user is shown", page.inner_text("#u-name") == "tester")
        check("hidden helper elements stay hidden", page.is_hidden("#devchip") and page.is_hidden("#dbrow") and page.is_hidden("#projbar"))

        # ---- analysis and markers
        page.fill("#code", CODE)
        analyze(page)
        page.wait_for_selector("#ftable table tbody tr")
        check("findings are listed", page.locator("#ftable tbody tr").count() == 7, page.locator("#ftable tbody tr").count())
        check("violations are marked in the editor gutter", page.locator("#gutter .gl.has").count() == 5, page.locator("#gutter .gl.has").count())
        check("lines are tinted behind the code", page.locator("#backdrop .bl.required, #backdrop .bl.advisory").count() == 5)
        shot(page, "01-analyze")
        page.locator("#ftable tbody tr", has_text="octal").first.click()
        check("clicking a finding selects its line in the editor", page.evaluate("document.getElementById('code').value.slice(document.getElementById('code').selectionStart, document.getElementById('code').selectionEnd)") == "int mask = 010;")

        # ---- export menu
        page.click("#exp-toggle")
        check("the export menu opens", page.is_visible("#exp-list"))
        with page.expect_download() as download:
            page.click("#exp-comp-html")
        check("the compliance report downloads", download.value.suggested_filename.startswith("compliance-"), download.value.suggested_filename)
        report = Path(download.value.path()).read_text()
        check("the report has its sections", "Guideline Compliance Summary" in report and "Deviation register" in report)
        check("the menu closes after a choice", page.is_hidden("#exp-list"))
        page.click("#exp-toggle"); page.mouse.click(5, 450)
        check("clicking elsewhere closes the menu", page.is_hidden("#exp-list"))
        with page.expect_download() as sarif:
            page.click("#exp-toggle"); page.click("#exp-sarif")
        check("SARIF downloads and is valid JSON", json.loads(Path(sarif.value.path()).read_text())["version"] == "2.1.0")

        # ---- triage
        page.locator("#ftable tbody tr", has_text="octal").first.locator("button[title^='False positive']").click()
        page.wait_for_selector("#tri-dialog:not([hidden])")
        page.fill("#tri-note", "browser test")
        page.click("#tri-form button[type=submit]")
        page.wait_for_selector("#tri-dialog", state="hidden")
        page.wait_for_selector("#ftable .vtag.false-positive")
        check("a false positive verdict appears next to the finding", page.locator("#ftable .vtag.false-positive").count() == 1)
        page.click("#miss-btn")
        page.wait_for_selector("#tri-dialog:not([hidden])")
        check("the missed violation dialog starts as a false negative", page.input_value("#tri-kind") == "false-negative")
        page.fill("#tri-rule", "15.5"); page.fill("#tri-note", "should be reported")
        page.click("#tri-form button[type=submit]")
        page.wait_for_selector("#tri-dialog", state="hidden")
        page.click("#nav-triage")
        page.wait_for_selector("#tri-body tr")
        check("the Triage tab lists both cases", page.locator("#tri-body tr").count() == 2, page.locator("#tri-body tr").count())
        check("the per-rule table shows them", page.locator("#tri-rules tr").count() == 2)
        shot(page, "02-triage")
        page.click("#tri-run")
        wait_until(page, "document.getElementById('tri-run').textContent === 'Re-run all'")
        page.click("#tri-promote")  # confirm dialog
        page.click("#nav-analyze")

        # ---- baseline: edit, analyse again, new / unchanged / fixed
        page.click("button:has-text('Save as baseline')")
        page.wait_for_selector("text=Update baseline")
        page.fill("#code", CODE + "int added = 077;\n")
        check("editing marks the result stale", page.is_visible("#stale-banner"))
        analyze(page)
        page.wait_for_selector(".card:has-text('new')")
        check("the baseline bar reports new findings", "new" in page.inner_text("#result") and page.locator(".newtag").count() >= 1)
        check("a verdict still follows its violation after the edit", page.locator("#ftable .vtag.false-positive").count() == 1)
        shot(page, "03-baseline")

        # ---- compare with cppcheck
        if not page.is_disabled("#xcheck"):
            page.check("#xcheck")
            analyze(page)
            page.wait_for_selector(".xtag, .card:has-text('cppcheck agrees')")
            page.click("#nav-compare")
            page.wait_for_selector("#cmp-ours .crow")
            check("the Compare tab fills the three panes", page.locator("#cmp-ours .crow").count() > 0 and page.locator("#cmp-misra .crow").count() > 0)
            page.locator("#cmp-ours .crow").first.click()
            page.wait_for_selector("#cmp-detail:not([hidden])")
            check("selecting a row shows the source excerpt", page.locator("#cmp-detail .excerpt .ln.target").count() == 1)
            shot(page, "04-compare")
            page.click("#nav-analyze")
        else:
            print("  (cppcheck not available: compare flow skipped)")

        # ---- zip project: upload, edit with autosave, analyse, markers in project mode
        buf = io.BytesIO()
        with zipfile.ZipFile(buf, "w") as z:
            z.writestr("fw/src/main.c", '#include "hal.h"\nint main(void)\n{\n    int m = 0755;\n    hal_init(m);\n    return 0;\n}\n')
            z.writestr("fw/src/hal.c", '#include "hal.h"\nint hal_init(int v)\n{\n    return v;\n}\n')
            z.writestr("fw/inc/hal.h", "int hal_init(int v);\n")
        zpath = Path(data) / "fw.zip"
        zpath.write_bytes(buf.getvalue())
        page.set_input_files("#zip", str(zpath))
        page.wait_for_selector("#projbar:not([hidden])")
        check("the project panel lists the files", page.locator("#pj-files .pj-file").count() == 3, page.locator("#pj-files .pj-file").count())
        page.locator("#pj-files .pj-file", has_text="main.c").click()
        wait_until(page, "document.getElementById('code').value.includes('0755')")
        check("project files are editable", page.is_editable("#code"))
        analyze(page)
        page.wait_for_selector("#ftable table tbody tr")
        page.locator("#ftable tbody tr", has_text="octal").first.click()
        wait_until(page, "document.querySelectorAll('#gutter .gl.has').length > 0")
        # clicking a finding reloads the file from the server; type straight away to exercise the race

        check("violations are marked in the code of a project file", page.locator("#gutter .gl.has").count() >= 1)
        shot(page, "05-project")
        page.fill("#code", '#include "hal.h"\nint main(void)\n{\n    int m = 755;\n    hal_init(m);\n    return 0;\n}\n')
        wait_until(page, "document.getElementById('pj-save').textContent === 'Saved'")
        check("edits autosave", page.inner_text("#pj-save") == "Saved")
        analyze(page)
        check("the edited project no longer reports the octal constant", "octal constant" not in page.inner_text("#result"))
        with page.expect_download() as zdl:
            page.click("#pj-download")
        with zipfile.ZipFile(zdl.value.path()) as z:
            check("the downloaded project contains the edit", b"755" in z.read("src/main.c") and b"0755" not in z.read("src/main.c"),
                  (z.namelist(), z.read("src/main.c")))

        # ---- publish to the shared history
        answers = ["e2e-fw", "main"]
        page.on("dialog", lambda d: d.accept(answers.pop(0)) if answers else d.dismiss())
        page.click("#exp-publish")
        page.wait_for_selector(".toast.show")
        page.click("#nav-history")
        page.wait_for_selector("#h-projects .hproj")
        check("the published project appears in the History tab", "e2e-fw" in page.inner_text("#h-projects"))
        page.click("#h-projects .hproj")
        page.wait_for_selector("#h-detail svg.chart")
        check("the trend chart is drawn", page.locator("#h-detail svg.chart polyline").count() == 3)
        shot(page, "06-history")

        # ---- rules tab, theme, narrow window
        page.click("#nav-rules")
        page.wait_for_selector("#rulebody tr")
        check("the rule catalogue lists all rules", page.locator("#rulebody tr").count() == 143, page.locator("#rulebody tr").count())
        page.click("#theme")
        dark = page.evaluate("getComputedStyle(document.body).backgroundColor")
        check("the theme toggle changes the colours", dark != "rgb(243, 245, 248)", dark)
        shot(page, "07-rules-dark")
        page.click("#theme")
        page.click("#nav-analyze")
        page.set_viewport_size({"width": 700, "height": 900})
        check("a narrow window has no horizontal page scroll", page.evaluate("document.documentElement.scrollWidth <= window.innerWidth + 1"),
              page.evaluate("[document.documentElement.scrollWidth, window.innerWidth]"))
        shot(page, "08-narrow")

        # ---- logout
        page.set_viewport_size({"width": 1440, "height": 900})
        page.click("#logout")
        page.wait_for_selector("#login:not([hidden])")
        check("logging out returns to the login page", page.is_visible("#login-form"))
        page.reload()
        page.wait_for_selector("#login:not([hidden])")
        check("the session is gone after a reload", page.is_visible("#login-form"))

        check("no JavaScript errors in the whole session", not errors, errors[:5])
        browser.close()


if __name__ == "__main__":
    sys.exit(main())
