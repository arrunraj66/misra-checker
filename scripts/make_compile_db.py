#!/usr/bin/env python3
"""Write a compile_commands.json for a plain source tree (no build system needed).

Every .c file becomes a translation unit; the root and every directory that holds
a .c or .h file go on the include path (the same rule the web GUI uses for zip
projects).

    make_compile_db.py PROJECT_DIR [--std c99] [-D NAME[=VAL]]... [--compiler clang-14]
Writes PROJECT_DIR/compile_commands.json (absolute paths, valid where it is run).
"""
import argparse
import json
import shutil
import sys
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("project", type=Path)
    parser.add_argument("--std", default="c99", choices=["c90", "c99", "c11", "c17"])
    parser.add_argument("-D", dest="defines", action="append", default=[])
    parser.add_argument("--compiler", default=None)
    args = parser.parse_args()

    root = args.project.resolve()
    compiler = args.compiler or shutil.which("clang-14") or shutil.which("clang") or "cc"
    sources = sorted(p for p in root.rglob("*.c") if "build" not in p.relative_to(root).parts)
    if not sources:
        print(f"no .c files under {root}", file=sys.stderr)
        return 1
    dirs = {root} | {p.parent for p in root.rglob("*") if p.suffix in (".c", ".h")}
    flags = [f"-std={args.std}", *(f"-D{d}" for d in args.defines),
             *(f"-I{d}" for d in sorted(dirs))]
    database = [{"directory": str(root), "file": str(s),
                 "arguments": [compiler, *flags, "-c", str(s)]} for s in sources]
    out = root / "compile_commands.json"
    out.write_text(json.dumps(database, indent=1))
    print(f"wrote {out} ({len(sources)} translation units)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
