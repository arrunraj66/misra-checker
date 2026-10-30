"""Compiler profiles: make Clang see the code the way the project's compiler does.

misra-checker always parses with Clang. A project built with another compiler (GCC,
arm-none-eabi-gcc, ...) differs in the target (type sizes, endianness), the system
headers (newlib instead of glibc) and a few version macros. A profile probes the
real compiler once and turns the answers into Clang flags:

    --target=<triple of the compiler>          (-dumpmachine)
    -nostdinc -isystem <its include dirs>       (-E -v), without GCC's own
                                                 builtin-header directories, plus
                                                 Clang's builtin headers
    -U/-D __GNUC__, __GNUC_MINOR__, ...         (-dM -E), so version checks match

Compilers that use proprietary syntax (IAR, Keil/armcc, TI) cannot be parsed by
Clang; use the free-form flags (for example -D__root=) to map their keywords.

Only compilers from an allowlist are ever executed, never a path chosen by a
web client, and user flags must match FLAG_RE.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from dataclasses import dataclass, field
from pathlib import Path

CANDIDATES = [
    "gcc", "cc", "arm-none-eabi-gcc", "aarch64-linux-gnu-gcc", "aarch64-none-elf-gcc",
    "arm-linux-gnueabihf-gcc", "riscv64-unknown-elf-gcc", "riscv32-unknown-elf-gcc",
    "riscv64-linux-gnu-gcc", "avr-gcc", "msp430-elf-gcc", "powerpc-linux-gnu-gcc",
    "mips-linux-gnu-gcc", "xtensa-esp32-elf-gcc", "clang", "clang-14",
]

# Flags a user may add. Anything that can read files, load code or change what is
# being analysed beyond the project's own compiler settings is refused.
FLAG_RE = re.compile(
    r"^(-D[A-Za-z_]\w*(=[^\s;|&$`\\\"']*)?"
    r"|-U[A-Za-z_]\w*"
    r"|-m(cpu|arch|abi|fpu|float-abi|tune)=[\w.+-]+"
    r"|-m(thumb|arm|big-endian|little-endian|soft-float|hard-float|no-unaligned-access|unaligned-access)"
    r"|-f(freestanding|signed-char|unsigned-char|short-enums|no-short-enums|short-wchar|no-builtin)"
    r"|-std=(c90|c99|c11|c17|gnu90|gnu99|gnu11|gnu17))$")

TIMEOUT = 15


@dataclass
class Compiler:
    id: str
    path: str
    family: str = "unknown"        # "clang" | "gcc"
    version: str = ""
    triple: str = ""
    include_dirs: list[str] = field(default_factory=list)
    gnuc: dict[str, str] = field(default_factory=dict)
    usable: bool = False
    reason: str = ""

    @property
    def label(self) -> str:
        name = self.version or self.id
        return f"{name} [{self.triple}]" if self.triple else name

    def describe(self) -> dict:
        return {"id": self.id, "label": self.label, "family": self.family, "triple": self.triple,
                "usable": self.usable, "reason": self.reason}


def validate_flags(text: str) -> list[str]:
    """Split a user flag string and reject anything outside FLAG_RE."""
    flags = text.split()
    if len(flags) > 40:
        raise ValueError("too many compiler flags")
    for flag in flags:
        if not FLAG_RE.match(flag):
            raise ValueError(f"compiler flag not allowed: {flag}")
    return flags


def _run(args: list[str], stdin: str = "") -> subprocess.CompletedProcess:
    return subprocess.run(args, input=stdin, capture_output=True, text=True, timeout=TIMEOUT)


def clang_resource_include(parser: str) -> str:
    out = _run([parser, "-print-resource-dir"]).stdout.strip()
    return str(Path(out) / "include") if out else ""


def target_supported(parser: str, triple: str) -> bool:
    proc = _run([parser, f"--target={triple}", "-fsyntax-only", "-x", "c", "-"], stdin="int x;\n")
    return proc.returncode == 0


def probe(identifier: str, path: str, parser: str) -> Compiler:
    """Run the compiler a few times to learn its family, target and include dirs."""
    compiler = Compiler(identifier, path)
    try:
        version = _run([path, "--version"])
        if version.returncode != 0:
            compiler.reason = "does not run"
            return compiler
        first = (version.stdout or version.stderr).splitlines()[0].strip()
        compiler.version = first
        low = first.lower()
        compiler.family = "clang" if "clang" in low else "gcc" if ("gcc" in low or "g++" in low or "free software foundation" in version.stdout.lower()) else "unknown"
        if compiler.family == "unknown":
            compiler.reason = "not a GCC or Clang compatible compiler (proprietary front ends cannot be parsed by Clang)"
            return compiler
        compiler.triple = _run([path, "-dumpmachine"]).stdout.strip()
        search = _run([path, "-E", "-x", "c", "-v", "-"]).stderr
        inside = False
        for line in search.splitlines():
            if "#include <...> search starts here" in line:
                inside = True
            elif "End of search list" in line:
                inside = False
            elif inside and line.startswith(" "):
                compiler.include_dirs.append(line.strip().replace(" (framework directory)", ""))
        macros = _run([path, "-dM", "-E", "-x", "c", "-"]).stdout
        for name in ("__GNUC__", "__GNUC_MINOR__", "__GNUC_PATCHLEVEL__"):
            m = re.search(rf"^#define {name} (\d+)$", macros, re.M)
            if m:
                compiler.gnuc[name] = m[1]
        if not compiler.triple:
            compiler.reason = "could not determine the target"
        elif not target_supported(parser, compiler.triple):
            compiler.reason = f"this Clang build does not support the target {compiler.triple}"
        else:
            compiler.usable = True
    except (OSError, subprocess.TimeoutExpired, IndexError) as exc:
        compiler.reason = f"probe failed: {exc}"
    return compiler


def discover(parser: str, extra: dict[str, str] | None = None) -> dict[str, Compiler]:
    """The parser itself as 'default', then allowlisted extras, then compilers on PATH."""
    found: dict[str, Compiler] = {}
    default = probe("default", parser, parser)
    found["default"] = default
    seen = {str(Path(parser).resolve())}
    for identifier, path in (extra or {}).items():
        found[identifier] = probe(identifier, path, parser)
        seen.add(str(Path(path).resolve()))
    for name in CANDIDATES:
        located = shutil.which(name)
        if not located:
            continue
        real = str(Path(located).resolve())
        if real in seen:
            continue
        seen.add(real)
        found[name] = probe(name, located, parser)
    return found


def clang_flags(compiler: Compiler, parser: str) -> list[str]:
    """Flags that make `parser` (Clang) imitate `compiler`."""
    if not compiler.usable:
        raise ValueError(f"compiler '{compiler.id}' is not usable: {compiler.reason}")
    if compiler.id == "default":
        return []
    flags = [f"--target={compiler.triple}"]
    if compiler.family == "gcc":
        flags.append("-nostdinc")
        builtin = clang_resource_include(parser)
        if builtin:
            flags += ["-isystem", builtin]
        for directory in compiler.include_dirs:
            # GCC's own builtin-header directories use GCC-only intrinsics; Clang's are used instead.
            if "/lib/gcc/" in directory:
                continue
            flags += ["-isystem", directory]
        for name, value in compiler.gnuc.items():
            flags += [f"-U{name}", f"-D{name}={value}"]
    return flags
