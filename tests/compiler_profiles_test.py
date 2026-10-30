#!/usr/bin/env python3
"""scripts/compiler_profiles.py: probing, Clang flags and the flag allowlist."""
import json
import os
import stat
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))
import compiler_profiles as cp  # noqa: E402

CHECKER, CLANG = sys.argv[1], sys.argv[2]
failures = []


def check(name, condition, detail=""):
    if not condition:
        failures.append(f"{name}: {detail}")


with tempfile.TemporaryDirectory() as tmp:
    tmp = Path(tmp)
    inc = tmp / "fake-newlib" / "include"
    inc.mkdir(parents=True)
    (inc / "fakelib.h").write_text("#define FAKE_NEWLIB 1\n")
    gccint = tmp / "lib" / "gcc" / "arm-none-eabi" / "10" / "include"
    gccint.mkdir(parents=True)

    # A stand-in for arm-none-eabi-gcc that answers the three probes.
    fake = tmp / "arm-none-eabi-gcc"
    fake.write_text(f"""#!/bin/sh
case "$*" in
  *--version*) echo "arm-none-eabi-gcc (Fake Toolchain) 10.3.1"; echo "Copyright Free Software Foundation";;
  *-dumpmachine*) echo arm-none-eabi;;
  *-dM*) printf '#define __GNUC__ 10\\n#define __GNUC_MINOR__ 3\\n#define __GNUC_PATCHLEVEL__ 1\\n';;
  *"-E -x c -v"*) cat >&2 <<EOF
#include <...> search starts here:
 {gccint}
 {inc}
End of search list.
EOF
  ;;
esac
""")
    fake.chmod(fake.stat().st_mode | stat.S_IEXEC)

    found = cp.discover(CLANG, {"arm-fake": str(fake)})
    check("default is first and clang", list(found)[0] == "default" and found["default"].family == "clang")
    check("default usable", found["default"].usable, found["default"].reason)
    arm = found["arm-fake"]
    check("fake probed as gcc", arm.family == "gcc" and arm.usable, (arm.family, arm.reason))
    check("triple", arm.triple == "arm-none-eabi", arm.triple)
    check("include dirs", arm.include_dirs == [str(gccint), str(inc)], arm.include_dirs)
    check("gnuc macros", arm.gnuc == {"__GNUC__": "10", "__GNUC_MINOR__": "3", "__GNUC_PATCHLEVEL__": "1"}, arm.gnuc)
    flags = cp.clang_flags(arm, CLANG)
    check("target flag", "--target=arm-none-eabi" in flags and "-nostdinc" in flags, flags)
    check("fake include added, gcc-internal dir dropped",
          str(inc) in flags and str(gccint) not in flags, flags)
    check("version macros", "-D__GNUC__=10" in flags and "-U__GNUC__" in flags, flags)
    check("default needs no flags", cp.clang_flags(found["default"], CLANG) == [])

    # The flags really change what Clang sees: 4-byte long and the fake headers.
    src = tmp / "t.c"
    src.write_text("""#include <fakelib.h>
#if __GNUC__ != 10
#error wrong compiler version
#endif
_Static_assert(sizeof(long) == 4, "arm-none-eabi has a 32-bit long");
int v = FAKE_NEWLIB;
""")

    def analyze(extra):
        db = [{"directory": str(tmp), "file": str(src), "arguments": [CLANG, "-std=c99", *extra, "-c", str(src)]}]
        (tmp / "compile_commands.json").write_text(json.dumps(db))
        return subprocess.run([CHECKER, "analyze", "--compile-commands", str(tmp / "compile_commands.json")],
                              capture_output=True, text=True)

    ok = analyze(flags)
    check("project compiles with the ARM profile", "results may be incomplete" not in ok.stderr, ok.stderr[-400:])
    plain = analyze([])
    check("same file fails with the host profile", "results may be incomplete" in plain.stderr)

    # A tool that is not GCC/Clang is refused with an explanation.
    odd = tmp / "iccarm"
    odd.write_text("#!/bin/sh\necho 'IAR C/C++ Compiler 9.20'\n")
    odd.chmod(odd.stat().st_mode | stat.S_IEXEC)
    iar = cp.probe("iar", str(odd), CLANG)
    check("unsupported front end refused", not iar.usable and "proprietary" in iar.reason, iar.reason)
    try:
        cp.clang_flags(iar, CLANG)
        check("unusable compiler raises", False)
    except ValueError:
        pass

    # Unsupported target: the explanation names it.
    xt = tmp / "xtensa-esp32-elf-gcc"
    xt.write_text(fake.read_text().replace("arm-none-eabi", "xtensa-esp32-elf"))
    xt.chmod(xt.stat().st_mode | stat.S_IEXEC)
    unsupported = cp.probe("xt", str(xt), CLANG)
    check("target Clang lacks is reported", not unsupported.usable and "does not support the target" in unsupported.reason,
          unsupported.reason)

# flag allowlist
good = "-DDEBUG=1 -DSTM32F4 -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffreestanding -fshort-enums -std=gnu99"
check("good flags accepted", len(cp.validate_flags(good)) == 9)
for bad in ["-include /etc/passwd", "-Xclang -load", "-fplugin=x.so", "-I/etc", "-isystem /", "-DX=$(id)", "-D X", "-o /tmp/x",
            "-fsanitize=address", "--sysroot=/", "-x c++", "-DA=`id`"]:
    try:
        cp.validate_flags(bad)
        check(f"rejected: {bad}", False, "accepted")
    except ValueError:
        pass

if failures:
    print("\n".join(failures))
    sys.exit(1)
print("compiler profile tests passed")
