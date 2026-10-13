# Independent validation of the rule detectors (first campaign)

**Status: evidence, not a conformance claim.** This campaign checks the detectors
against an *independent oracle* (code whose authors publish their MISRA C:2012
deviations). The checking was done by the same team that wrote the detectors, so it
does **not** satisfy the independent-verification requirement of the roadmap. That
still needs a person outside the development team, and the licensed MISRA example
suite. What this campaign does is find real defects early and fix them, and leave a
reproducible, measurable baseline.

## Oracle

[FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) at commit
`8be86d4a24fd4091f8f4192018423ab590f408db`. Its maintainers state conformance to
MISRA C:2012 (checked with Coverity), publish every remaining violation as an inline
`coverity[misra_c_2012_rule_X_Y_violation]` marker (61 markers in 6 files), and list
project-wide deviations in `examples/coverity/coverity_misra.config` (22 entries: 17 rules
and 5 directives).

Configuration: the authors' own Coverity setup (template port, their
`FreeRTOSConfig.h`, `pdTRUE`/`pdFALSE` renamed as in their build), C99, 8
translation units, about 17,000 lines. The Coverity setup of the authors is the
reference, so only what that tool and configuration can see is covered.

Reproduce: `python3 scripts/run_freertos_oracle.py --work-dir /tmp/frk --report-dir out`
(clones the pinned commit; `scripts/validate_oracle.py` does the comparison).

## Method

For every finding in a rule that is **not** deviated project-wide:

- next to a marker for that rule: **documented** (the two tools agree);
- no marker: **unexplained**, a suspected false positive until a person triages it.

For every marker: **detected**, **missed** (suspected false negative), or **not compiled
here** (the marked line is in an inactive `#if` group in this configuration, found from
`clang -E` line markers; those cannot be judged).

A second, weaker opinion comes from cppcheck's MISRA add-on
(`scripts/validate_against_cppcheck.py`). It disagrees with the Coverity-backed claim
in several places (for example it reports rules 5.5, 12.3 and 20.2 in code claimed
compliant), so it is used only as supporting evidence.

## Result

| | First run | After fixes |
|---|---|---|
| Findings in rules not deviated project-wide that the authors do not document | 123 | 14 |
| Documented deviations in compiled code that the checker detects | 32 of 37 | 32 of 37 (86%) |

(The first run also included `heap_3.c`, which wraps `malloc`/`free` and is outside the
authors' analysed scope; it was dropped. Its Rule 21.3 findings were among the 123.)

| Rule | Findings (not project-deviated) | Documented (agree) | Unexplained | Documented deviations in compiled code | Detected | Missed | Not compiled here |
|---|---|---|---|---|---|---|---|
| 8.4 | 1 | 1 | 0 | 1 | 1 | 0 | 2 |
| 8.6 | 2 | 0 | 2 | 0 | 0 | 0 | 4 |
| 10.5 | 3 | 0 | 3 | 0 | 0 | 0 | 0 |
| 11.1 | 0 | 0 | 0 | 2 | 0 | 2 | 0 |
| 11.3 | 18 | 11 | 7 | 11 | 11 | 0 | 1 |
| 11.5 | 20 | 20 | 0 | 20 | 20 | 0 | 11 |
| 13.5 | 2 | 0 | 2 | 0 | 0 | 0 | 0 |
| 14.3 | 0 | 0 | 0 | 2 | 0 | 2 | 0 |
| 18.1 | 0 | 0 | 0 | 1 | 0 | 1 | 0 |
| 21.6 | 0 | 0 | 0 | 0 | 0 | 0 | 6 |

Rows with no findings, markers or unexplained entries are omitted; the full data is in
`docs/validation/freertos-oracle.json` and the list of unexplained lines with source in
`docs/validation/freertos-unexplained.csv`.

## Defects found and fixed

Seven false-positive classes in the detectors, each now covered by a regression fixture
under `tests/fixtures/obs`:

| Rule | Defect | Fixture |
|---|---|---|
| 5.2, 5.4 | Used 31 significant characters for internal identifiers and macro names; C99 and later use 63 (external identifiers stay at 31). 79 false findings. | `r5_2_c99_ok`, `r5_4_c99_ok`, updated `_bad` |
| 10.3, 10.4 | Essential type of arithmetic and `?:` expressions used the promoted C type (`unsigned char + unsigned char` became `int`), so `uint8 = a + b`, `s = c ? t : s` and `a == (b - c)` were reported. | `r10_3_composite_ok`, `r10_3_conditional_ok`, `r10_4_composite_ok` |
| 11.1 | A function pointer assigned to a type differing only by an attribute such as `noreturn` was treated as a conversion. | `r11_1_noreturn_ok` |
| 11.2 | Comparing or assigning a null pointer constant to a pointer to an incomplete type (opaque handles) was reported. | `r11_2_null_ok` |
| 11.9 | `NULL` passed through the argument of another macro was not recognised as the `NULL` macro. | `r11_9_macro_arg_ok` |
| 20.1 | An `#include` inside an inactive `#if` group was reported, and inactive code counted as code before an include. | `r20_1_inactive_ok` |
| 20.9 | Integer suffixes in `#if` conditions (`1U`) were read as undefined identifiers. | `r20_9_suffix_ok` |

## Still open (14 unexplained findings, 5 missed deviations)

These are **not** claimed to be correct. Each needs a decision from the independent
reviewer against the licensed rule text.

| Rule | Count | Assessment |
|---|---|---|
| 8.6 | 2 | Artifact: the definitions live in `heap_3.c`, which I removed from the scope. Not a detector defect. |
| 10.5 | 3 | `(char) 0x00`: cast of an integer constant to a character type. Strict reading flags it; the oracle tool does not. Undecided. |
| 11.3 | 7 | Casts between object pointers and `int8_t *`/`uint8_t *`. A strict reading flags them. Neither Coverity nor cppcheck does, so this may be an exception for character-type pointers in the licensed text. Undecided; do not change until checked against the rule text. |
| 13.5 | 2 | Volatile reads on the right of `&&`. The kernel's own comments say volatile reads there are rewritten "to ensure compliance with Rule 13.5", so these are probably true positives that Coverity does not report. |
| 14.3 | 2 missed | Documented invariant controlling expressions that involve configuration constants; the detector only handles fully constant conditions. Known limitation. |
| 11.1 | 2 missed | `(void) function_pointer;` casts. The authors treat them as violations; the detector does not flag a cast to `void`. Undecided; many code bases use this idiom. |
| 18.1 | 1 missed | Possible out-of-bounds index found by flow analysis. Not decidable by the current syntactic detector. |

## Limits of this evidence

- One code base, one configuration, about 17,000 lines, C99 only; no directives (the
  checker implements rules only).
- The oracle covers only what Coverity reported. A rule absent from the markers is not
  proof of compliance (for example a rule Coverity does not check).
- 24 of the 61 documented deviations are in code not compiled in this configuration and
  were not judged.
- Only 19 of the 143 rules produced any finding in this corpus, so this says little
  about the other 124. Their evidence is still only the per-rule fixtures
  (`docs/rule-validation.md`).
- Whole-program rules depend on analysing the full program.

## Next steps

1. Independent reviewer triages the open items above against the licensed text and
   records decisions (then fix or document).
2. Obtain the licensed MISRA example suite and run it through a harness like
   `validate_oracle.py` (expected results per line) to get per-rule false-negative and
   false-positive rates.
3. Add further code bases, other build configurations (the 24 uncompiled markers need
   the other `#if` branches), a second compiler front end, and C90 mode.
4. Set per-rule release thresholds (roadmap section 14) and track them in CI.
