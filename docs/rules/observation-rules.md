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

### Second batch

| Rule | Diagnostic | Detection contract |
|---|---|---|
| 1.2 | `misra-c2012-1.2-language-extension` | Report use of compiler extensions: statement expressions, inline assembly, computed goto, label addresses, case ranges, typeof, omitted conditional operand and explicit attributes. |
| 2.1 | `misra-c2012-2.1-unreachable-code` | Report a statement that directly follows return, break, continue or goto in the same block (labels excluded). |
| 2.2 | `misra-c2012-2.2-no-effect-statement` | Report an expression statement with no side effects (a cast to void is accepted). |
| 2.3 | `misra-c2012-2.3-unused-typedef` | Report a typedef declared in the main file that no type in the translation unit refers to. |
| 2.4 | `misra-c2012-2.4-unused-tag` | Report a named struct, union or enum tag declared in the main file that is never referred to as a type. |
| 2.5 | `misra-c2012-2.5-unused-macro` | Report a macro defined in the main file that is never expanded or tested. |
| 5.1 | `misra-c2012-5.1-external-identifiers-not-distinct` | Report external identifiers longer than 31 characters that are identical in their first 31 characters but differ afterwards (translation-unit view). |
| 5.2 | `misra-c2012-5.2-scope-identifiers-not-distinct` | Report identifiers in one scope longer than 31 characters that are identical in their first 31 characters but differ afterwards. |
| 5.3 | `misra-c2012-5.3-hides-outer-declaration` | Report a block-scope or parameter declaration whose name matches a file-scope or enclosing-scope identifier. |
| 5.4 | `misra-c2012-5.4-macro-name-not-distinct` | Report macro names longer than 31 characters that differ only after the 31st character. |
| 5.5 | `misra-c2012-5.5-identifier-matches-macro-name` | Report a declared identifier that has the same name as a macro defined in the main file. |
| 5.6 | `misra-c2012-5.6-typedef-name-not-unique` | Report a typedef whose name is reused by another declaration (a tag naming the same type is accepted). |
| 5.7 | `misra-c2012-5.7-tag-name-not-unique` | Report a tag whose name is reused by another declaration (a typedef naming the same type is accepted). |
| 7.2 | `misra-c2012-7.2-missing-unsigned-suffix` | Report an integer literal of unsigned type that is spelled without a u or U suffix. |
| 7.4 | `misra-c2012-7.4-string-literal-to-non-const` | Report a string literal that converts to a pointer to non-const char outside subscript and dereference contexts. |
| 8.8 | `misra-c2012-8.8-missing-static-on-redeclaration` | Report a redeclaration of an internal-linkage function or object that omits the static specifier. |
| 8.9 | `misra-c2012-8.9-single-function-object` | Report a file-scope static object that is referenced from exactly one function. |
| 8.12 | `misra-c2012-8.12-duplicate-implicit-enumerator` | Report an implicitly valued enumerator whose value equals another enumerator in the same list. |
| 9.3 | `misra-c2012-9.3-partial-array-initializer` | Report an array initializer list that supplies fewer elements than the array size, other than the single-zero form. |
| 9.4 | `misra-c2012-9.4-duplicate-initialization` | Report an element designated more than once in one initializer list. |
| 9.5 | `misra-c2012-9.5-unsized-array-designated-init` | Report an array of unspecified size initialized with array designators. |
| 10.1 | `misra-c2012-10.1-inappropriate-operand-type` | Subset: report Boolean or enumerated operands of arithmetic operators, unsigned unary minus, and non-unsigned or non-constant operands of bitwise and shift operators. |
| 10.3 | `misra-c2012-10.3-essential-type-narrowing` | Report assignment or initialization of a non-constant value to an object of a different essential type category or narrower width. |
| 10.4 | `misra-c2012-10.4-essential-type-mismatch` | Report binary arithmetic, relational, equality and bitwise operators whose non-constant operands have different essential type categories. |
| 10.5 | `misra-c2012-10.5-inappropriate-cast` | Subset: report casts to or from Boolean, to character, to enumeration of another type, and character to floating. |
| 11.2 | `misra-c2012-11.2-incomplete-pointer-conversion` | Report a conversion between a pointer to an incomplete type and any other pointer or integer type. |
| 11.6 | `misra-c2012-11.6-void-pointer-integer-cast` | Report an explicit cast between pointer-to-void and an integer type. |
| 11.9 | `misra-c2012-11.9-null-pointer-constant-not-null-macro` | Report a null pointer constant that is not spelled through the NULL macro. |
| 12.2 | `misra-c2012-12.2-shift-out-of-range` | Report a constant shift count that is negative or not smaller than the essential width of the left operand. |
| 13.1 | `misra-c2012-13.1-initializer-side-effect` | Report an initializer-list element that has side effects. |
| 13.4 | `misra-c2012-13.4-assignment-result-used` | Report an assignment whose result is used by an enclosing expression or condition. |
| 14.1 | `misra-c2012-14.1-float-loop-counter` | Report a for loop whose increment expression modifies a floating-point variable. |
| 14.3 | `misra-c2012-14.3-invariant-condition` | Subset: report if, do, conditional-operator, and false while/for conditions that evaluate to a constant outside macro expansions. |
| 14.4 | `misra-c2012-14.4-non-boolean-condition` | Report an if, loop or conditional-operator condition that is not essentially Boolean (relational, equality, logical, negation, or _Bool). |
| 16.3 | `misra-c2012-16.3-missing-break` | Strict reading: report a switch clause whose last statement is not a break (or a block ending in break). |
| 16.7 | `misra-c2012-16.7-boolean-switch-expression` | Report a switch whose controlling expression is essentially Boolean. |
| 17.3 | `misra-c2012-17.3-implicit-function-declaration` | Report a call to an implicitly declared (non-builtin) function. |
| 17.4 | `misra-c2012-17.4-missing-return-value` | Report a return without a value in a non-void function, or a non-void function body that is not structurally guaranteed to end in a return. |
| 17.6 | `misra-c2012-17.6-static-array-parameter` | Report an array parameter declared with the static keyword inside the brackets. |
| 20.1 | `misra-c2012-20.1-include-after-code` | Report an include directive preceded by anything other than directives or comments. |
| 20.2 | `misra-c2012-20.2-invalid-header-name-characters` | Report an include header name containing a quote, backslash, comment marker or (angle form) double quote. |
| 20.4 | `misra-c2012-20.4-macro-named-keyword` | Report a macro whose name is a language keyword. |
| 20.7 | `misra-c2012-20.7-macro-parameter-unparenthesized` | Report a function-like macro using a parameter in the replacement list without adjacent parentheses or commas (operands of # and ## and member names are excluded). |
| 20.11 | `misra-c2012-20.11-stringify-before-paste` | Report a macro in which a # operand is immediately followed by ##. |
| 20.12 | `misra-c2012-20.12-macro-parameter-operand-and-expanded` | Report a macro parameter used both as an operand of # or ## and elsewhere. |
| 20.13 | `misra-c2012-20.13-invalid-directive` | Report a line starting with # whose directive name is not a recognised preprocessing directive. |
| 21.1 | `misra-c2012-21.1-reserved-macro-name` | Report #define or #undef of defined or an underscore-reserved name. |
| 21.2 | `misra-c2012-21.2-reserved-identifier` | Report a declaration using an underscore-reserved identifier or defining a standard library function name. |
| 21.11 | `misra-c2012-21.11-tgmath-include` | Report an include of the tgmath header. |
| 21.12 | `misra-c2012-21.12-banned-function` | Report a call to one of the listed functions: floating-point exception handling function from fenv.h called (matched by callee name). Names: `feclearexcept`, `fegetexceptflag`, `feraiseexcept`, `fesetexceptflag`, `fetestexcept`. |
| 22.5 | `misra-c2012-22.5-file-object-dereferenced` | Report dereference of a FILE pointer or member access through it. |

## Known limitations (all rules above)

- Syntactic detectors only; no semantic or whole-program reasoning beyond what is stated. Rule 17.2 sees only calls visible in one translation unit.
- Library-function rules (21.x) match by callee name, not by declaring header, and miss calls through function pointers.
- Findings are attributed to the macro use site; macro-expansion stacks are not reported.
- Rule semantics here are engineering interpretations (e.g. 4.1 treats a full three-digit octal escape as terminated, 6.1 allows int, unsigned int and _Bool). They need independent review against the licensed MISRA text before any compliance claim.
- Tested against Clang/LLVM 18 in development; the repository baseline remains LLVM 14 and needs regression testing there.

- Second-batch approximations: the 10.x rules use a simplified essential-type model and are marked as subsets; 14.3 skips conditions inside macros; 16.3 requires a literal break (return/continue/goto endings are flagged); 17.4 uses a structural end-of-function check rather than a control-flow graph; 5.1 and 5.2 look at one translation unit only; 20.7 treats an adjacent parenthesis or comma on both sides as enclosure; 2.3/2.4 count any type reference as use.
- Not implemented because Clang cannot represent the construct in a successfully parsed program: Rule 20.14 (a conditional directive closed in another file is a hard preprocessor error).
