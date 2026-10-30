# Baselines, stable violation identity and the CI gate

Professional checkers let a team adopt a rule set on existing code and still insist that nothing
gets worse. That needs two things: a way to recognise the *same* violation after the code around it
changed, and a saved reference state to compare against.

## Violation fingerprint
A violation is identified by **rule + file + the text of its line (whitespace collapsed) + its
occurrence number** among identical lines of the same rule in that file. So:

- adding or removing code above it, reformatting the line, or editing other functions keeps it the same;
- changing the line itself, renaming the file or moving the code to another file makes it a new violation.

The fingerprint (`fp`) is in every GUI result, in the CSV export and in baseline files.

## In the GUI
After an analysis the results show a baseline bar.

1. **Save as baseline** remembers the current findings for this workspace (a project is identified by its
   name, pasted files by their file names). Baselines are private to the user and keep the last 20
   snapshots as a history.
2. Edit the code and analyse again. Cards show **new**, **unchanged** and **fixed**; each new finding
   carries a *new* tag; chips filter new / unchanged; **Fixed since the baseline** lists what was removed.
   A badge says whether there is a new *required or mandatory* violation, which is what a CI gate would stop.
3. **Update baseline** accepts the current state; **Clear** removes the baseline.

Triage verdicts (true / false positive) are matched to violations by fingerprint, so they stay on the right
violation when code moves. Cases created before this existed fall back to the line number.

## On the command line and in CI
```bash
# once: accept the current state of the project
python3 scripts/misra_ci.py --compile-commands build/compile_commands.json \
    --baseline misra-baseline.json --update-baseline

# in CI: fail only when a NEW required or mandatory violation appears
python3 scripts/misra_ci.py --compile-commands build/compile_commands.json \
    --baseline misra-baseline.json --min-category required --deviations misra-deviations.json
```
Options: `--min-category advisory|required|mandatory` (default `required`), `--fail-on new|any|none`
(`new` when a baseline is given), `--sarif-out FILE`, `--json-out FILE`, `--source-root DIR` (baseline paths
are relative to it, so the file is portable between machines). Commit `misra-baseline.json`. Exit status:
0 passed, 1 gate failed, 2 the checker could not run. A GitHub Actions example is in
[docs/ci/github-actions.yml](ci/github-actions.yml).

Findings covered by an approved deviation never count against the gate.

## Limits
- Matching is by line text, not by program structure: two edits to the same line look like "fixed + new".
- The occurrence number shifts if an identical earlier line is deleted, which can swap two identical
  violations; the counts stay right.
- Baselines of pasted code depend on the file names used.
