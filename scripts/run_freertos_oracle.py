#!/usr/bin/env python3
"""Reproducible oracle validation against the FreeRTOS-Kernel (see docs/independent-validation.md).

Clones FreeRTOS-Kernel at a pinned commit, prepares it the way the authors' own
Coverity job does (template port, their FreeRTOSConfig.h, pdTRUE/pdFALSE renamed),
builds a compile database and runs validate_oracle.py.

    run_freertos_oracle.py --work-dir /tmp/frk [--checker build/misra-checker] [--report-dir out]
"""
import argparse
import glob
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

REPO = "https://github.com/FreeRTOS/FreeRTOS-Kernel.git"
COMMIT = "8be86d4a24fd4091f8f4192018423ab590f408db"
HERE = Path(__file__).resolve().parent


def run(*cmd, **kw):
    return subprocess.run(cmd, check=True, **kw)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--work-dir", type=Path, required=True)
    ap.add_argument("--checker", default=str(HERE.parent / "build" / "misra-checker"))
    ap.add_argument("--report-dir", type=Path, default=Path("oracle-report"))
    ap.add_argument("--clang", default=shutil.which("clang-14") or shutil.which("clang") or "clang")
    args = ap.parse_args()

    src = args.work_dir / "FreeRTOS-Kernel"
    if not src.exists():
        args.work_dir.mkdir(parents=True, exist_ok=True)
        run("git", "clone", "-q", REPO, str(src))
    run("git", "-C", str(src), "fetch", "-q", "origin", COMMIT)
    run("git", "-C", str(src), "checkout", "-q", "--force", COMMIT)
    run("git", "-C", str(src), "clean", "-fdq")

    # The kernel's Coverity job renames pdTRUE/pdFALSE in its own sources.
    sources = sorted(glob.glob(str(src / "*.c"))) + [str(src / "portable/template/port.c")]
    for path in sources:
        text = Path(path).read_bytes().replace(b"pdFALSE", b"pdFAIL").replace(b"pdTRUE", b"pdPASS")
        Path(path).write_bytes(text)

    flags = ["-std=c99", f"-I{src}/include", f"-I{src}/portable/template", f"-I{src}/examples/coverity"]
    db = [{"directory": str(src), "file": f, "arguments": [args.clang, *flags, "-c", f]} for f in sources]
    db_path = src / "compile_commands.json"
    db_path.write_text(json.dumps(db, indent=1))

    run(sys.executable, str(HERE / "validate_oracle.py"), "--checker", args.checker,
        "--compile-commands", str(db_path), "--source-root", str(src),
        "--deviation-config", str(src / "examples/coverity/coverity_misra.config"),
        "--report-dir", str(args.report_dir), "--name", "freertos")


if __name__ == "__main__":
    sys.exit(main())
