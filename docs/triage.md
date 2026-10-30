# Triage: turning what you see in the GUI into checker improvements

The loop: **edit and check, label what is right and wrong, replay, fix, keep the case as a test.**

## 1. Edit and check
Upload a project zip, click a file in the list and edit it in the editor. Changes autosave to the
server ("Saved" next to the buttons). **New file** adds a `.c` or `.h` to the project, **Download**
gives you the edited project as a zip. Press **Analyze project** (Ctrl+Enter) to check the edited
code. Violations are marked in the code (red / orange / blue dots and tinted lines).

## 2. Compare with cppcheck
Tick **Cross-check with cppcheck** and analyse, then open the **Compare** tab: three panes show
misra-checker, cppcheck's MISRA add-on and cppcheck's own checks (out-of-bounds, uninitialised
variables, ...). Click a row to highlight the same line in all three and see the source around it.
**Only differences** hides the lines where both tools agree.

## 3. Label
- **✓ / ✗ next to a finding**: true positive (the violation is real) or false positive (the checker is
  wrong). Add a note saying why.
- **+ Missed violation** above the editor (or **+ missed** on a line only cppcheck reports): the checker
  should have reported a rule on a line but did not (a false negative). The dialog starts on the line
  your cursor is on.

Each label is saved as a case with the code, the compiler settings and your note. The same verdict on
the same line text updates the case instead of duplicating it. Cases are private to their author;
administrators see all.

## 4. Replay and read the results
The **Triage** tab lists the cases and what each says about each rule (true / false positives, missed
violations, precision implied by your verdicts). **Re-run all** replays every case against the current
checker: a case shows **as labelled** when the checker now agrees with your verdict, **open** when it
does not. The **Rules** tab shows your verdicts next to each rule.

## 5. Fix and keep it
An open false positive or missed violation is a to-do for the checker. When it is fixed the case turns
into **as labelled**. Administrators use **Promote to tests** to copy cases into `tests/triage/`, where
`ctest` replays them (`scripts/validate_triage.py --check`):

- a case marked `fixed` that stops behaving as labelled **fails the build** (a regression);
- an `open` case is listed but does not fail it.

Command line: `python3 scripts/validate_triage.py [DIR] [--update]` (`--update` rewrites each case's
state). Export the cases as a zip from the Triage tab to share them.

## Limits
- Replay needs the code to be reproducible. A case keeps the file and as much of the project as fits
  (about 400 KB); larger projects keep the files nearest the finding and may miss headers.
- A case made with a non-default compiler profile depends on that compiler being installed; the runner
  skips such a case instead of failing it.
- A false-positive case says "no report of this rule on this line". If you later edit the code the line
  number moves, so label again on the new line.
- Labels are your judgement. A rule's real accuracy needs the licensed rule text and an independent
  reviewer (see `docs/independent-validation.md`).
