# Validation notes

## zlib 1.2.11 smoke run (engineering, not a conformance result)

All 15 library translation units (excluding the example programs) were analyzed
together with LLVM 14 and `-std=c99`: no crash, about 0.6 s, 2,416 findings
across 40 rules. Sampling the largest groups against the source:

- Plausible and consistent with the code style (K&R definitions, unbraced
  bodies, `0` as null, pointer arithmetic, chained assignments, `goto`):
  8.2, 11.9, 15.6, 15.1, 18.4, 13.4, 14.4, 7.4, 16.4, 17.8, 8.9, 5.9, 11.4.
- Reported for missing `<unistd.h>` under strict C99: 17.3.
- Found and fixed: references to compiler built-ins and implicit declarations
  (for example `va_start`) were reported by Rule 8.6 as having no definition.
  A regression fixture was added (`sys/r8_6_ok`).
- Open question: Rule 3.1 reports a URL (`http://`) inside a block comment.
  This follows a strict reading of the rule and has not been reviewed.
- Rule 8.3 reports a K&R definition whose parameter types are spelled with the
  underlying type while the prototype uses a typedef name; this is a strict
  reading and needs independent review.

This run checks robustness and obvious false positives only. It is not a
substitute for an independent oracle (for example a commercial checker or the
MISRA example suite) and says nothing about false negatives.
