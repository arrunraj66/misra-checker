# Choosing the project's compiler

misra-checker always parses code with Clang (LLVM 14). A project built with another
compiler can differ in the **target** (size of `long`, pointers, endianness), the
**system headers** (newlib on bare metal, glibc on Linux) and **compiler version
macros** (`__GNUC__`). Selecting the project's compiler makes Clang see the code the way
that compiler does. Nothing is built or linked; see "Why we don't compile".

## In the GUI

Pick the compiler in the **Compiler** list next to the language standard (the choice is
remembered), and add anything else the build passes in **Compiler flags**, for example
`-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -DSTM32F4`. Use the GNU standards (`gnu99`,
`gnu11`, `gnu17`) when the project is built that way. The result shows "Compiler
profile: …" so a report states what was used.

The list contains the Clang that ships with the tool and every supported compiler found
on the server's `PATH` (`gcc`, `arm-none-eabi-gcc`, `aarch64-linux-gnu-gcc`,
`riscv64-unknown-elf-gcc`, `avr-gcc`, …). A compiler that cannot be used is shown greyed
out with the reason as a tooltip.

To offer a compiler that is not on `PATH`, start the server with
`--compiler ID=/path/to/compiler` (repeatable) or set
`MISRA_GUI_COMPILERS="arm=/opt/arm/bin/arm-none-eabi-gcc;riscv=/opt/rv/bin/riscv64-unknown-elf-gcc"`.
Only compilers named this way or found on `PATH` are ever run; a web user cannot supply
a path.

## On the command line

```bash
python3 scripts/make_compile_db.py ./myproject --compiler arm-none-eabi-gcc \
    "--flags=-mcpu=cortex-m4 -mthumb -DSTM32F4" --std gnu99
misra-checker analyze --compile-commands ./myproject/compile_commands.json
```
Write `--flags=...` with an equals sign. The same profile is used by `convert`, because it
reads the same database.

## What a profile does

It runs the compiler three times and turns the answers into Clang flags:

| Probe | Used for |
|---|---|
| `-dumpmachine` | `--target=<triple>`: type sizes, endianness |
| `-E -x c -v -` | its system include directories (`-nostdinc -isystem …`). GCC's own builtin-header directory is replaced by Clang's, because those headers use GCC-only intrinsics |
| `-dM -E` | `__GNUC__`, `__GNUC_MINOR__`, `__GNUC_PATCHLEVEL__`, so `#if __GNUC__ >= 9` behaves the same |

Allowed extra flags: `-D`, `-U`, `-mcpu=`, `-march=`, `-mabi=`, `-mfpu=`, `-mfloat-abi=`,
`-mtune=`, `-mthumb`, `-marm`, endianness and float switches, `-ffreestanding`,
`-fsigned-char`, `-funsigned-char`, `-fshort-enums`, `-fno-builtin` and `-std=`.
Anything that can read a file, load code or add include paths (`-include`, `-I`,
`-Xclang`, `-fplugin`, `--sysroot`, …) is refused; use the project's *Extra include
dirs* field for extra include paths.

## Limits

- **Proprietary compilers** (IAR, Keil/armcc, TI, Green Hills) use syntax Clang cannot
  parse (`__root`, `__irq`, `#pragma` forms). They cannot be selected. Map their keywords
  away with defines, for example `-D__root=` and `-D__irq=`, and analyse with the closest
  GCC/Clang profile. The results then depend on how faithful those defines are.
- **Targets this Clang lacks**: LLVM 14 has no Xtensa (ESP32) target, so
  `xtensa-esp32-elf-gcc` is listed as unavailable.
- **The compiler has to run where the server runs.** In Docker, mount the toolchain
  read-only and name it, for example in `docker-compose.yml`:
  `volumes: ["/opt/arm-gnu-toolchain:/opt/arm-gnu-toolchain:ro"]` and
  `MISRA_GUI_COMPILERS: arm=/opt/arm-gnu-toolchain/bin/arm-none-eabi-gcc`.
- **Compiler extensions** that Clang does not implement can still cause compile errors.
  Those are shown as a notice and as Rule 1.1 findings; the other rules still run.

## Why we don't compile

The rules need the parsed, type-checked program, not machine code. Building would need
every library and startup file, which embedded projects often cannot provide on a PC,
and would run the project's build steps on the server. The converter does compile: it
refuses an AI fix unless the file still compiles with the original flags, and it can run
your own build and test command.
