# Validation report (engineering, differential)

**Scope and honesty.** This is a differential comparison against one open
implementation (cppcheck 2.13 `misra.py`) plus robustness runs on real C code.
It is *not* a conformance result: cppcheck is not ground truth, the corpora are
small, no MISRA-licensed test suite was available, and nothing here replaces
independent validation or tool qualification. The aim was to find defects in
misra-checker and to understand where the two tools differ.

## Method

1. Corpora (built with `-std=c99`, LLVM 14): zlib 1.2.11 (15 files), Lua 5.4.4
   (32 files), bzip2 1.0.8 (7 files), cJSON 1.7.17 (1 file).
2. `scripts/validate_against_cppcheck.py` runs misra-checker once over a
   compile database and cppcheck's dump + `misra.py` per file, then compares the
   sets of `(file, line)` per rule. `scripts/summarize_validation.py` merges
   corpora. cppcheck runs one configuration (`--max-configs=1`); findings in
   project headers are included on both sides.
3. Robustness: every corpus analyzed three times; outputs compared byte for
   byte.

Reproduce:

```bash
python3 scripts/validate_against_cppcheck.py --checker build/misra-checker \
    --compile-commands zlib.json --out zlib_report.json
python3 scripts/summarize_validation.py report.md zlib=zlib_report.json lua=lua_report.json
```

## Robustness results

| Corpus | Result |
|---|---|
| zlib, Lua, bzip2, cJSON | no crashes; 0.2-0.8 s each; output identical across three runs (output is now sorted to guarantee this) |

## Defects found and fixed through this exercise

- Findings in project headers were dropped (for example unions in `deflate.h`,
  macro/identifier clashes defined in headers). Headers are now reported, once,
  except for findings that only make sense per translation unit.
- Rule 2.2 reported about 600 macro-generated `((void)0)` statements in Lua;
  void casts in parentheses and macro-expanded statements are now ignored.
- Rule 12.4 reported shifts that discard bits (`~0u << n`); only addition,
  subtraction and multiplication are checked now.
- Rule 5.5 only saw macros defined in the main file; all project macros are used.
- Rules 7.1-7.3 and 4.1 reported every macro use; they now report at the
  literal's spelling in project code.
- Rule 7.4 flagged string literals inside conditional expressions that end up
  in `const char *`.
- Rule 20.9 scanned comments and character literals, and flagged names guarded
  by `defined()` in the same condition.
- Rule 11.1 treated a null pointer constant compared with a function pointer as
  a conversion.
- Output order depended on container iteration order; it is now sorted.

## Per-rule comparison (zlib + Lua, exact file:line)

`agree/ours` = share of our findings cppcheck also reports (precision against
the oracle); `agree/cppcheck` = share of cppcheck's findings we also report
(recall against the oracle). A dash means no findings. Low values are not
automatically defects; see the interpretation below.

| Rule | misra-checker | cppcheck | both | agree/ours | agree/cppcheck |
|---|---:|---:|---:|---:|---:|
| 1.2 | 353 | 0 | 0 | 0.00 | - |
| 2.1 | 63 | 0 | 0 | 0.00 | - |
| 2.2 | 4 | 2 | 0 | 0.00 | 0.00 |
| 2.3 | 1 | 0 | 0 | 0.00 | - |
| 2.5 | 34 | 0 | 0 | 0.00 | - |
| 3.1 | 10 | 5 | 5 | 0.50 | 1.00 |
| 4.1 | 2 | 1 | 1 | 0.50 | 1.00 |
| 5.3 | 1 | 0 | 0 | 0.00 | - |
| 5.5 | 3 | 15 | 2 | 0.67 | 0.13 |
| 5.9 | 2 | 0 | 0 | 0.00 | - |
| 7.1 | 6 | 2 | 1 | 0.17 | 0.50 |
| 7.2 | 1 | 10 | 0 | 0.00 | 0.00 |
| 7.3 | 1 | 1 | 0 | 0.00 | 0.00 |
| 7.4 | 50 | 3 | 0 | 0.00 | 0.00 |
| 8.2 | 157 | 358 | 15 | 0.10 | 0.04 |
| 8.3 | 50 | 0 | 0 | 0.00 | - |
| 8.4 | 2 | 126 | 2 | 1.00 | 0.02 |
| 8.6 | 34 | 0 | 0 | 0.00 | - |
| 8.7 | 95 | 0 | 0 | 0.00 | - |
| 8.9 | 22 | 27 | 22 | 1.00 | 0.81 |
| 8.11 | 3 | 3 | 3 | 1.00 | 1.00 |
| 8.13 | 115 | 0 | 0 | 0.00 | - |
| 9.1 | 22 | 0 | 0 | 0.00 | - |
| 10.1 | 152 | 976 | 65 | 0.43 | 0.07 |
| 10.2 | 0 | 2 | 0 | - | 0.00 |
| 10.3 | 254 | 70 | 37 | 0.15 | 0.53 |
| 10.4 | 100 | 1651 | 92 | 0.92 | 0.06 |
| 10.5 | 117 | 0 | 0 | 0.00 | - |
| 10.6 | 27 | 13 | 3 | 0.11 | 0.23 |
| 10.7 | 7 | 8 | 0 | 0.00 | 0.00 |
| 10.8 | 316 | 301 | 214 | 0.68 | 0.71 |
| 11.1 | 3 | 2 | 2 | 0.67 | 1.00 |
| 11.2 | 34 | 25 | 25 | 0.74 | 1.00 |
| 11.3 | 367 | 321 | 321 | 0.87 | 1.00 |
| 11.4 | 11 | 28 | 9 | 0.82 | 0.32 |
| 11.5 | 105 | 76 | 76 | 0.72 | 1.00 |
| 11.6 | 2 | 1 | 1 | 0.50 | 1.00 |
| 11.8 | 23 | 114 | 23 | 1.00 | 0.20 |
| 11.9 | 186 | 111 | 109 | 0.59 | 0.98 |
| 12.1 | 672 | 666 | 542 | 0.81 | 0.81 |
| 12.2 | 0 | 78 | 0 | - | 0.00 |
| 12.3 | 447 | 324 | 256 | 0.57 | 0.79 |
| 13.3 | 377 | 235 | 221 | 0.59 | 0.94 |
| 13.4 | 190 | 93 | 89 | 0.47 | 0.96 |
| 13.5 | 180 | 56 | 56 | 0.31 | 1.00 |
| 14.2 | 13 | 9 | 8 | 0.62 | 0.89 |
| 14.3 | 2 | 0 | 0 | 0.00 | - |
| 14.4 | 854 | 420 | 394 | 0.46 | 0.94 |
| 15.1 | 99 | 99 | 99 | 1.00 | 1.00 |
| 15.2 | 13 | 13 | 13 | 1.00 | 1.00 |
| 15.3 | 5 | 9 | 5 | 1.00 | 0.56 |
| 15.4 | 19 | 22 | 19 | 1.00 | 0.86 |
| 15.5 | 378 | 973 | 0 | 0.00 | 0.00 |
| 15.6 | 1554 | 1584 | 1452 | 0.93 | 0.92 |
| 15.7 | 47 | 47 | 1 | 0.02 | 0.02 |
| 16.1 | 320 | 0 | 0 | 0.00 | - |
| 16.3 | 315 | 66 | 38 | 0.12 | 0.58 |
| 16.4 | 5 | 6 | 5 | 1.00 | 0.83 |
| 16.6 | 0 | 4 | 0 | - | 0.00 |
| 17.1 | 27 | 39 | 27 | 1.00 | 0.69 |
| 17.2 | 61 | 112 | 0 | 0.00 | 0.00 |
| 17.3 | 10 | 132 | 2 | 0.20 | 0.02 |
| 17.7 | 281 | 295 | 218 | 0.78 | 0.74 |
| 17.8 | 257 | 257 | 257 | 1.00 | 1.00 |
| 18.1 | 1 | 0 | 0 | 0.00 | - |
| 18.4 | 679 | 602 | 602 | 0.89 | 1.00 |
| 18.6 | 1 | 0 | 0 | 0.00 | - |
| 19.1 | 1 | 0 | 0 | 0.00 | - |
| 19.2 | 19 | 467 | 19 | 1.00 | 0.04 |
| 20.1 | 15 | 14 | 11 | 0.73 | 0.79 |
| 20.5 | 8 | 12 | 6 | 0.75 | 0.50 |
| 20.7 | 60 | 110 | 53 | 0.88 | 0.48 |
| 20.9 | 9 | 3 | 1 | 0.11 | 0.33 |
| 20.10 | 3 | 10 | 3 | 1.00 | 0.30 |
| 20.12 | 1 | 0 | 0 | 0.00 | - |
| 21.1 | 0 | 5 | 0 | - | 0.00 |
| 21.2 | 16 | 0 | 0 | 0.00 | - |
| 21.3 | 36 | 36 | 36 | 1.00 | 1.00 |
| 21.4 | 2 | 1 | 0 | 0.00 | 0.00 |
| 21.5 | 0 | 1 | 0 | - | 0.00 |
| 21.6 | 65 | 12 | 0 | 0.00 | 0.00 |
| 21.8 | 6 | 5 | 5 | 0.83 | 1.00 |
| 21.10 | 12 | 4 | 0 | 0.00 | 0.00 |
| 21.14 | 0 | 1 | 0 | - | 0.00 |
| 21.15 | 0 | 5 | 0 | - | 0.00 |
| 21.21 | 0 | 1 | 0 | - | 0.00 |
| 22.8 | 0 | 3 | 0 | - | 0.00 |
| 22.9 | 0 | 2 | 0 | - | 0.00 |
| 22.10 | 0 | 1 | 0 | - | 0.00 |

## Interpreting the disagreements

Rules where both tools agree closely (at least 0.85 in the table in both
directions, or identical sets): 15.1, 15.2, 15.4, 15.6, 17.8, 21.3, 8.11, 14.2.
Rules where we reproduce every cppcheck finding but also report more (agree/
cppcheck of 1.00, agree/ours of 0.7-0.9): 11.2, 11.3, 11.5, 18.4, 8.4.

Where we find more because we are stricter or cppcheck lacks the check:
1.2, 2.1, 2.5, 8.3, 8.6, 8.7, 8.13, 9.1, 10.5, 16.1, 21.2 (cppcheck has no or
different checks); 16.3 (we require a literal `break`; cppcheck accepts other
terminators); 13.5, 13.3, 14.4, 11.9 (broader matching).

Where cppcheck finds more:
- 10.1, 10.3, 10.4, 10.6-10.8: cppcheck models essential types more completely;
  ours is a documented subset. These are real false negatives of misra-checker.
- 8.2, 15.5, 15.7, 17.2: different *reporting position* (parameter line vs
  function name; each `return` vs the function; closing brace vs `if`) or
  granularity, not necessarily different verdicts. Counts for 15.7 match (47
  vs 47) even though lines do not.
- 8.4, 17.3, 12.2, 12.3, 19.2: manual spot checks suggest cppcheck false
  positives or different semantics (parenthesized declarators such as
  `(lua_checkstack)` hide the earlier prototype from cppcheck; declaration
  commas such as `unsigned n, m;` reported as comma operators; union use sites
  versus union definitions).
- 11.8, 11.4: cppcheck counts `(char *)"literal"` (string literals are
  unqualified `char[]` in C) and `(char *)0`; we do not for 11.8/11.4 (7.4 covers
  the literal case).
- 21.x: cppcheck also flags the `#include <stdio.h>` line itself; we flag calls.

## Not validated

- No independent oracle exists here for: 9.x, 13.2, 14.3, 16.x clause rules,
  18.1-18.3/18.6, 19.1, 20.6, 20.8, 20.14, 22.x, 5.x (identifier distinctness),
  and cross-translation-unit rules beyond what cppcheck reports for 8.x.
- Rules added in MISRA C:2012 Amendments 3/4 (21.14, 21.15, 21.21, 22.8-22.10)
  are reported by cppcheck but are outside this project's 143-rule baseline.
- False-negative rates are unknown. A seeded-defect (mutation) campaign and a
  licensed MISRA example suite are still required.
