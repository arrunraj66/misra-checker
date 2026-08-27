# MISRA C Checker and Controlled Converter

An industrial static-analysis and controlled-remediation project for MISRA C.

The initial engineering baseline is MISRA C:2012 with the compliance process from MISRA Compliance:2020. The architecture is designed for versioned rule packs and later qualification or certification for a selected functional-safety domain.

See [ROADMAP.md](ROADMAP.md) for the 78-week implementation and assurance program.

## Repository policy

- This repository must remain private until licensing and publication claims are approved.
- Do not commit licensed MISRA PDFs, copied guideline text, customer source code, credentials, compiler licences, or proprietary test corpora.
- A successful scan is not by itself a MISRA compliance claim.
- Automatic remediation is restricted to independently approved transformation classes.

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
