# Observation-based rule detectors

Original engineering notes; licensed MISRA wording remains authoritative and is not reproduced. Each rule below is implemented as a mapping from AST/preprocessor *observations* recorded by `src/clang_observations.cpp` to diagnostics. All are **Implemented; independent validation pending** and each has a `_bad` and `_ok` fixture under `tests/fixtures/obs/`.

| Rule | Diagnostic | Detection contract |
|---|---|---|
| 2.6 | `misra-c2012-2.6-unused-label` | Report a label in a function body that no goto statement or label-address expression references. |
| 2.7 | `misra-c2012-2.7-unused-parameter` | Report a named parameter of a function definition that is never referenced in the body. |
| 3.1 | `misra-c2012-3.1-comment-nested-marker` | Report a comment whose body contains a comment-opening marker (/* or //). |
| 3.2 | `misra-c2012-3.2-comment-line-splice` | Report a line comment that is continued onto the next line by a trailing backslash. |
| 4.1 | `misra-c2012-4.1-unterminated-escape` | Report an octal or hexadecimal escape sequence in a string or character literal that is not followed by the closing quote or another escape (a complete three-digit octal escape counts as terminated). |
| 4.2 | `misra-c2012-4.2-trigraph` | Report a trigraph sequence found in the raw text of the main file. |
| 6.1 | `misra-c2012-6.1-bitfield-type` | Report a bit-field whose canonical type is not int, unsigned int or _Bool. |
| 6.2 | `misra-c2012-6.2-signed-single-bit-field` | Report a named bit-field of signed integer type with width one. |
| 7.1 | `misra-c2012-7.1-octal-constant` | Report an integer literal spelled with a leading zero followed by further digits. |
| 7.3 | `misra-c2012-7.3-lowercase-l-suffix` | Report an integer or floating literal whose suffix uses lowercase l. |
| 8.2 | `misra-c2012-8.2-unprototyped-or-unnamed` | Report a function declared without a prototype or with an unnamed parameter. |
| 8.4 | `misra-c2012-8.4-no-prior-declaration` | Report a definition of an externally visible function or object that has no earlier declaration (main excluded). |
| 8.10 | `misra-c2012-8.10-external-inline` | Report an inline function definition that is not declared static. |
| 8.11 | `misra-c2012-8.11-extern-array-no-size` | Report an extern array declaration with no explicit size. |
| 8.14 | `misra-c2012-8.14-restrict-qualifier` | Report any object, parameter or member with a restrict-qualified type. |
| 11.1 | `misra-c2012-11.1-function-pointer-conversion` | Report a cast that converts to or from a function pointer type, other than function-designator decay. |
| 11.3 | `misra-c2012-11.3-pointer-type-mismatch-cast` | Report an explicit cast between pointers to differing object types (void pointers excluded). |
| 11.4 | `misra-c2012-11.4-pointer-integer-cast` | Report an explicit cast between a pointer and an integer type (null constants excluded). |
| 11.5 | `misra-c2012-11.5-void-pointer-to-object` | Report a conversion from pointer-to-void to pointer-to-object, implicit or explicit. |
| 11.8 | `misra-c2012-11.8-cast-removes-qualifier` | Report an explicit pointer cast that drops const or volatile from the pointee type. |
| 12.3 | `misra-c2012-12.3-comma-operator` | Report every use of the comma operator. |
| 13.5 | `misra-c2012-13.5-side-effect-in-logical-rhs` | Report a right operand of && or || that has side effects (including calls and volatile access). |
| 13.6 | `misra-c2012-13.6-sizeof-side-effect` | Report a sizeof operand expression that has side effects. |
| 15.5 | `misra-c2012-15.5-multiple-exit` | Report a function with more than one return statement, or whose single return is not the last statement of the body. |
| 15.6 | `misra-c2012-15.6-body-not-compound` | Report an if, else, loop or switch body that is not a compound statement (else-if chains are allowed). |
| 15.7 | `misra-c2012-15.7-missing-final-else` | Report an if/else-if chain that does not end in a plain else. |
| 16.2 | `misra-c2012-16.2-nested-switch-label` | Report a case or default label that is not a top-level clause of its switch body. |
| 16.4 | `misra-c2012-16.4-switch-no-default` | Report a switch statement without a default label. |
| 16.5 | `misra-c2012-16.5-default-not-first-or-last` | Report a switch whose default clause is neither the first nor the last clause. |
| 16.6 | `misra-c2012-16.6-switch-too-few-clauses` | Report a switch statement with fewer than two clauses. |
| 17.1 | `misra-c2012-17.1-stdarg-use` | Report use of va_arg or the va_start/va_end/va_copy built-ins. |
| 17.2 | `misra-c2012-17.2-recursion` | Report a function that can reach itself through direct or mutual calls visible in the translation unit. |
| 17.7 | `misra-c2012-17.7-unused-call-result` | Report a call to a non-void function used as an expression statement (a cast to void is accepted). |
| 17.8 | `misra-c2012-17.8-parameter-modified` | Report assignment, compound assignment, increment or decrement applied directly to a function parameter. |
| 18.4 | `misra-c2012-18.4-pointer-arithmetic` | Report +, -, += or -= with a pointer-typed operand. |
| 18.5 | `misra-c2012-18.5-pointer-nesting` | Report a declaration whose type has more than two levels of pointer nesting. |
| 18.7 | `misra-c2012-18.7-flexible-array-member` | Report a structure whose last member is an array of unspecified size. |
| 18.8 | `misra-c2012-18.8-variable-length-array` | Report a declaration of variable-length array type. |
| 19.2 | `misra-c2012-19.2-union-declared` | Report the definition of a union type. |
| 20.5 | `misra-c2012-20.5-undef` | Report every #undef directive. |
| 20.10 | `misra-c2012-20.10-stringify-or-paste-operator` | Report a macro definition that uses the # or ## operator. |
| 21.3 | `misra-c2012-21.3-banned-function` | Report a call to one of the listed functions: dynamic memory allocation function called (matched by callee name). Names: `malloc`, `calloc`, `realloc`, `free`, `aligned_alloc`. |
| 21.4 | `misra-c2012-21.4-banned-function` | Report a call to one of the listed functions: setjmp/longjmp facility used (matched by callee name). Names: `setjmp`, `_setjmp`, `longjmp`, `sigsetjmp`, `siglongjmp`. |
| 21.5 | `misra-c2012-21.5-banned-function` | Report a call to one of the listed functions: signal handling function called (matched by callee name). Names: `signal`, `raise`. |
| 21.6 | `misra-c2012-21.6-banned-function` | Report a call to one of the listed functions: standard input/output function called (matched by callee name). Names: `printf`, `fprintf`, `sprintf`, `snprintf`, `vprintf`, `vfprintf`, `vsprintf`, `vsnprintf`, `scanf`, `fscanf`, `sscanf`, `vscanf`, `vfscanf`, `vsscanf`, `fopen`, `freopen`, `fclose`, `fflush`, `fread`, `fwrite`, `fgetc`, `fgets`, `fputc`, `fputs`, `getc`, `getchar`, `gets`, `putc`, `putchar`, `puts`, `ungetc`, `fseek`, `ftell`, `rewind`, `fgetpos`, `fsetpos`, `clearerr`, `feof`, `ferror`, `perror`, `remove`, `rename`, `tmpfile`, `tmpnam`, `setbuf`, `setvbuf`. |
| 21.7 | `misra-c2012-21.7-banned-function` | Report a call to one of the listed functions: string-to-number conversion from the atox family called (matched by callee name). Names: `atof`, `atoi`, `atol`, `atoll`. |
| 21.8 | `misra-c2012-21.8-banned-function` | Report a call to one of the listed functions: abort, exit, getenv or system called (matched by callee name). Names: `abort`, `exit`, `getenv`, `system`. |
| 21.9 | `misra-c2012-21.9-banned-function` | Report a call to one of the listed functions: bsearch or qsort called (matched by callee name). Names: `bsearch`, `qsort`. |
| 21.10 | `misra-c2012-21.10-banned-function` | Report a call to one of the listed functions: standard time/date function called (matched by callee name). Names: `time`, `clock`, `difftime`, `mktime`, `strftime`, `ctime`, `asctime`, `gmtime`, `localtime`, `wcsftime`. |

## Known limitations (all rules above)

- Syntactic detectors only; no semantic or whole-program reasoning beyond what is stated. Rule 17.2 sees only calls visible in one translation unit.
- Library-function rules (21.x) match by callee name, not by declaring header, and miss calls through function pointers.
- Findings are attributed to the macro use site; macro-expansion stacks are not reported.
- Rule semantics here are engineering interpretations (e.g. 4.1 treats a full three-digit octal escape as terminated, 6.1 allows int, unsigned int and _Bool). They need independent review against the licensed MISRA text before any compliance claim.
- Tested against Clang/LLVM 18 in development; the repository baseline remains LLVM 14 and needs regression testing there.
