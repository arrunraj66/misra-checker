# Final batch of detectors and coverage statement

Original engineering notes; licensed MISRA wording remains authoritative and is not reproduced.

With this batch every one of the 143 rule classes has a detector. **"Implemented" does not mean a complete decision procedure.** Many rules are undecidable or depend on information outside one translation unit; their detectors report a documented subset of violations and will miss others. Rules whose contract starts with "Subset" below are in that category; so are the 10.x family, 12.1, 13.x, 14.x, 16.3, 17.4, 18.x and 22.x from earlier batches. Nothing here is a compliance claim.

| Rule | Diagnostic | Detection contract |
|---|---|---|
| 1.1 | `misra-c2012-1.1-language-violation` | Report every compiler error (syntax or constraint violation) located in the main file. |
| 1.3 | `misra-c2012-1.3-undefined-behavior` | Subset: report constant division by zero, constant array indexes outside the array, dereference of a null pointer constant and out-of-range constant shifts. |
| 8.1 | `misra-c2012-8.1-implicit-type` | Report a declaration with no written type specifier (implicit int). |
| 8.13 | `misra-c2012-8.13-pointer-could-be-const` | Subset: report a pointer parameter to non-const whose every use in the function is provably read-only. |
| 9.2 | `misra-c2012-9.2-brace-elision` | Report an aggregate initializer that omits the braces of a nested aggregate (the {0} idiom is exempt). |
| 10.2 | `misra-c2012-10.2-character-arithmetic` | Report character operands used inappropriately in addition or subtraction. |
| 10.6 | `misra-c2012-10.6-composite-assigned-wider` | Report a composite arithmetic expression assigned to an object of wider essential type. |
| 10.7 | `misra-c2012-10.7-composite-operand-narrower` | Report a composite expression used as operand with a wider non-constant operand. |
| 10.8 | `misra-c2012-10.8-composite-cast-widened` | Report a cast of a composite expression to a wider or different essential type. |
| 11.7 | `misra-c2012-11.7-pointer-float-cast` | Report a cast between a pointer and a floating type. Clang rejects such casts, so the check is reached only through extensions. |
| 12.1 | `misra-c2012-12.1-implicit-precedence` | Strict reading: report a binary operator with an unparenthesized binary operand of a different precedence level. |
| 12.4 | `misra-c2012-12.4-constant-wraparound` | Report a constant unsigned addition, subtraction, multiplication or shift whose exact result does not fit the type. |
| 13.2 | `misra-c2012-13.2-unsequenced-access` | Subset: report a scalar variable that is modified and also read or modified without a sequence point in one full expression. |
| 13.3 | `misra-c2012-13.3-increment-with-side-effect` | Report an increment or decrement that is not a whole expression statement and shares its full expression with another side effect. |
| 16.1 | `misra-c2012-16.1-malformed-switch` | Report any violation of the switch-structure constituents (labels at top level, break per clause, default placement, clause count, non-Boolean controlling expression). |
| 17.5 | `misra-c2012-17.5-array-argument-too-small` | Report an argument smaller than the stated size of its array parameter, or a null argument for one. |
| 18.1 | `misra-c2012-18.1-pointer-arithmetic-out-of-bounds` | Subset: report constant offsets or constant indexes that fall outside the declared array. |
| 18.2 | `misra-c2012-18.2-pointer-subtraction-different-objects` | Subset: report subtraction of pointers derived from two different declared arrays. |
| 18.3 | `misra-c2012-18.3-pointer-comparison-different-objects` | Subset: report relational comparison of pointers derived from two different declared arrays. |
| 18.6 | `misra-c2012-18.6-address-of-local-escapes` | Subset: report returning, or storing in longer-lived storage, the address of an automatic object. |
| 19.1 | `misra-c2012-19.1-overlapping-copy` | Subset: report self-assignment and copy calls whose source and destination are the same expression. |
| 20.3 | `misra-c2012-20.3-include-macro-operand` | Report an include whose operand is not written as a header name; the macro may still expand correctly, so the finding is only possible. |
| 20.6 | `misra-c2012-20.6-embedded-directive` | Report a preprocessing directive embedded in macro arguments (reported by the compiler as an extension warning). |
| 20.8 | `misra-c2012-20.8-if-condition-not-boolean` | Subset: report an identifier-free #if/#elif condition that evaluates to a value other than 0 or 1. |
| 20.9 | `misra-c2012-20.9-undefined-identifier-in-if` | Report an identifier in an #if/#elif condition that is not a defined macro at that point. |
| 20.14 | `misra-c2012-20.14-conditional-directive-unbalanced` | Report a conditional directive that is not closed in the file where it was opened (the compiler rejects these). |
| 22.1 | `misra-c2012-22.1-resource-not-released` | Subset: report a local object holding an allocation or open stream that is neither released nor handed on in the function. |
| 22.2 | `misra-c2012-22.2-invalid-free` | Subset: report freeing the address of an object, an array or a string literal, and straight-line double free. |
| 22.3 | `misra-c2012-22.3-file-opened-twice` | Subset: report a file opened twice in one function with at least one write mode. |
| 22.4 | `misra-c2012-22.4-write-to-read-only-stream` | Subset: report a write to a stream opened only for reading in the same function. |
| 22.6 | `misra-c2012-22.6-use-after-close` | Subset: report a stream used after fclose later in the same block without reassignment. |

## Mechanisms

- **Compiler diagnostics (1.1, 20.6, 20.14):** a forwarding diagnostic consumer records compile errors in the main file, the embedded-directive extension warning (enabled explicitly) and unbalanced conditional-directive errors. When a translation unit does not compile, the CLI still prints these findings and exits 1 (2 if there are none).
- **Expression-order analysis (13.2, 13.3):** each full expression is scanned for reads and writes of scalar variables, with `&&`, `||`, `?:` and `,` treated as sequence points.
- **Preprocessor conditions (20.8, 20.9):** `#if`/`#elif` text is evaluated only when it contains no identifiers; identifiers are checked against the macro table at that point.
- **Resource rules (22.x):** straight-line, intra-function patterns around `malloc`/`fopen`/`free`/`fclose`; anything handed to another function or stored elsewhere is treated as released or escaped.

## Not exercisable in the test suite

- Rule 11.7: Clang rejects pointer/floating casts, so there is no violating fixture (only a compliant one).
- Rule 8.1 targets C90; it is exercised here under C99 where Clang accepts implicit int with a warning.

## Triage on zlib

A run over zlib 1.2.11 completed without crashes (143 rules, 0.8 s). Rule 12.1 (strict precedence reading, 335 findings) and 13.3 (276 findings, mostly pointer post-increments in macros) are noisy by design. A false positive in Rule 10.7 (unary minus on a constant such as `-1` counted as a composite operand) was fixed.
