# MISRA C Checker and Controlled Converter

An industrial static-analysis and controlled-remediation project for MISRA C.

The initial engineering baseline is MISRA C:2012 with the compliance process from MISRA Compliance:2020. The architecture is designed for versioned rule packs and later qualification or certification for a selected functional-safety domain.

See [ROADMAP.md](ROADMAP.md) for the 78-week implementation and assurance program.

## Repository policy

- This public repository contains only original implementation code, engineering
  metadata, and paraphrased analysis plans. Licensed normative MISRA content
  must remain outside the repository.
- Do not commit licensed MISRA PDFs, copied guideline text, customer source code, credentials, compiler licences, or proprietary test corpora.
- A successful scan is not by itself a MISRA compliance claim.
- Automatic remediation is restricted to independently approved transformation classes.

## MISRA C:2012 rule structures

The repository now has one C++ translation unit and one concrete `Rule` class
for each of the 143 MISRA C:2012 rules. Every file begins with its rule ID,
engineering intent, analysis expansion, normative-source notice, and current
implementation status. See the complete metadata catalog in
[docs/rule-implementation-status.md](docs/rule-implementation-status.md).

```text
include/misra/rule.hpp                 common rule contract and metadata
include/misra/c2012/rule_factories.hpp factory declaration for every rule
rules/c2012/rule_01_01.cpp             separate Rule 1.1 structure
...
rules/c2012/rule_22_06.cpp             separate Rule 22.6 structure
src/rule_registry.cpp                  constructs the complete rule pack
tests/rule_registry_test.cpp           validates count, uniqueness and totals
```

Rule 15.1 now has the first Clang-backed detector. The remaining 142 classes are
scaffolds returning `EvaluationStatus::NotImplemented`. Rule 15.1 is implemented
but has not completed independent validation; the product must not yet be
represented as qualified, certified, or compliance-ready. Each detector will
advance independently through specification, implementation, positive/negative
tests, traceability, review, and validation.

List the registered structures with:

```bash
./build/misra-checker --list-rules
```

## Analyze a C project

The prototype currently requires LLVM and Clang 14 development packages. It
reads the exact compiler invocations from `compile_commands.json`, preserving
the target flags, include paths, defines, language version, and other build
context used to parse each translation unit.

For a CMake-based target project, generate the database with:

```bash
cmake -S /path/to/c-project -B /path/to/c-project/build \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

Analyze every file in that database:

```bash
./build/misra-checker analyze \
  --compile-commands /path/to/c-project/build
```

Or analyze one selected translation unit:

```bash
./build/misra-checker analyze \
  --compile-commands /path/to/c-project/build/compile_commands.json \
  --file /path/to/c-project/source.c
```

Exit code `0` means no findings from implemented rules, `1` means findings were
reported, `2` means the frontend could not complete analysis, and `64` means
the command line was invalid. A zero exit code covers only implemented rules;
it is not a project-level MISRA compliance result.

## Verified build commits

Use the wrapper below when a change should be committed only after its verification command succeeds:

```bash
./scripts/verified-build-commit.sh "Short commit message" -- <build-and-test-command>
```

Example once the CMake build exists:

```bash
./scripts/verified-build-commit.sh "Implement parser adapter" -- \
  bash -c 'cmake -S . -B build -G Ninja && cmake --build build && ctest --test-dir build --output-on-failure'
```

The wrapper:

1. runs the supplied build/test command;
2. stops without staging or committing if it fails;
3. rejects prohibited secret or licensed-document filenames;
4. stages only allow-listed project paths;
5. performs staged-diff hygiene checks;
6. creates a local Git commit only when staged changes exist.

It intentionally does not push automatically. Pushing should remain an explicit action, and protected branches should require CI.
