# Cross-checking with cppcheck

An independent second opinion: the GUI can run cppcheck's MISRA C:2012 add-on on the same
files as misra-checker and show where the two agree.

## Use

Tick **Cross-check with cppcheck** (next to the compiler flags) and analyse. The box is
disabled when the server has no cppcheck; the Docker image includes it. It works for pasted
code, several files and zip projects. It adds time: cppcheck runs on every `.c` file (up to
300 files, four at a time, 3 minutes per file).

The result shows:

- cards: **cppcheck agrees**, **only here**, **only cppcheck**;
- a **cppcheck** column on each finding: ✓ cppcheck (the same rule is reported on the same
  line), only here, or nothing when the add-on does not implement that rule;
- findings that **only cppcheck** reports, merged into the table;
- filter chips for the three groups; CSV, JSON and the HTML report carry the column.

From the command line use `scripts/validate_against_cppcheck.py` (per-rule statistics for a
whole project) or `scripts/cppcheck_cross.py` as a library.

## How to read it

- **Not ground truth.** cppcheck 2.7's add-on implements about 130 rule checks, many of them
  approximations. A finding in only one tool is a lead to inspect, not proof that either
  tool is wrong. In the FreeRTOS validation (`docs/independent-validation.md`) cppcheck
  reported rules 5.5, 12.3 and 20.2 many times in code whose authors state it is compliant.
- **Matching is exact**: same file, same line, same rule. A finding that spans lines or sits
  inside a macro may be reported on a different line by each tool.
- **Rules the add-on does not implement** have no counterpart and are never counted as
  disagreement.
- **Same inputs for both**: include paths, defines and the language standard come from the
  compile command; for an ARM or other 32-bit target profile cppcheck is told the platform
  has 32-bit pointers. Compiler-specific headers are not given to cppcheck.
- **Nothing is written to your folders**: cppcheck runs on symbolic links inside a temporary
  directory.

## Gaps this has already shown

- Octal constants inside `#define` lines (Rule 7.1) were reported by cppcheck and missed here. It was
  recorded as a missed-violation case and fixed (`tests/triage/fn-7-1-octal-in-define.json`).

Use the **Compare** tab for a side-by-side view and the triage buttons to record what you find
(see `docs/triage.md`).
