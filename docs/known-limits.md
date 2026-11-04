# Known limits of implemented detectors

Original engineering notes. Each limit is covered by a fixture so a change in
behaviour is visible.

## Rule 14.4 (controlling expression)
Uses the shared essential-type model (see the Section 10 notes below): a
condition is acceptable when its essential category is Boolean (`_Bool` values;
relational, equality and logical operators; `!`; `<stdbool.h>` `true`/`false`;
comma by its right operand; `?:` when both result operands are Boolean).
Integer promotions are looked through.

## Rule 13.4 (assignment result)
The result counts as used when its enclosing context consumes it: operands,
call arguments, conditions, return, initializers and the last statement of a
GNU statement expression. It does not count as used as an expression statement,
a `for` init/increment, a cast to `void`, or the left operand of a comma. The
value of a comma or `?:` expression is followed outward, so `a = 1, b = 2;` is
clean and `x = c ? (y = 1) : 0;` is not. Increment and decrement operators are
not assignments for this rule.

## Rule 8.3 (consistent declarations)
Types are compared as element type plus array bounds. `int a[]` is compatible
with `int a[4]` and `int a[5]`, while `int a[4]` and `int a[5]` conflict; every
declaration is compared with every earlier one because compatibility is not
transitive. Parameter names are compared positionally. Function types are
compared as canonical text, so equal function-pointer types spelled through
different typedefs agree, but compatible-yet-distinct types (for example enums
and their underlying integer type) are reported.

## Section 10 rules (essential types) — 10.1, 10.2, 10.3, 10.4, 10.6, 10.7, 10.8
The model lives in `src/essential_type.{hpp,cpp}` and was written
independently from the rule intent. **Rule 10.5 is deliberately not
implemented**: its cast table could not be reconstructed with confidence, and a
guessed table would be worse than a gap. Validate every table below against the
licensed MISRA text before relying on a result.

Model:
* Categories: Boolean, Character (plain `char`), Enum (named enumerations and
  typedef-named anonymous ones), Signed, Unsigned, Floating. Pointers,
  aggregates, `void` and wide characters are `Other`; operations involving
  `Other` operands are not checked.
* Integer promotions and implicit conversions are looked through; an explicit
  cast takes the target type. `<stdbool.h>` `true`/`false` are Boolean.
  Constants of an unnamed enumeration are `int`, whatever type the compiler
  chose underneath. Bit-fields use their declared width.
* The result of an arithmetic or bitwise operator takes the type of its
  operands (the wider if both agree); a constant operand adopts the other
  operand's type. This reproduces the "essential, not promoted" behaviour.

Interpretations and leniencies (each is a place a licensed review may differ):
* 10.1: a non-negative signed *constant* is accepted as an operand of shifts,
  `~`, `&`, `|`, `^`. Operands of `Other` category are accepted.
* 10.3: assignment, initialization, argument passing to a prototyped function
  and `return` are all treated as assignment. Initializer lists are not
  checked. Compound assignments are checked by 10.1/10.4, not 10.3. An integer
  constant may initialize an object of the same category if it fits, or an
  unsigned object if non-negative and it fits; it may **not** initialize a
  Boolean, Character or Floating object (so `char c = 0;` and `float f = 1.0;`
  are reported).
* 10.4: `?:` result operands and compound assignments count as operators that
  perform the usual arithmetic conversions; shifts do not.
* 10.6/10.7/10.8: a *composite* expression is a non-constant arithmetic,
  bitwise, shift or unary `+ - ~` result. A cast result, comparison, call,
  member access and `?:` are not composite.

## Rules 5.2 and 5.3 (scopes)
Block scopes are tracked in the frontend: file scope, function scope (the
parameters and the outermost block share one scope), nested blocks, `for`
statements, prototype scope, and one scope per struct/union for members.
* Only project identifiers are modelled. A local that hides a name declared in
  a system header (for example a variable called `index`) is not reported.
* An outer declaration hides only what follows it. Within one file this is
  decided by source order; across files a header is assumed to precede its
  includer.
* Block-scope function declarations and `extern` declarations re-declare an
  existing entity and are not treated as hiding it.
* Parameters named inside a function *type* (function-pointer typedefs and
  declarators) are in their own prototype scope and are not recorded.

## Section 11 rules (pointer conversions) — 11.1-11.6, 11.8, 11.9
Implemented from the rule intent on Clang cast nodes; validate against the
licensed text before relying on a result.
* **11.7 is not implemented.** A cast between an object pointer and a floating
  type is a constraint violation that Clang rejects before analysis, so a
  detector could never fire. Such a file is reported as an analysis failure
  (exit code 2), not as a rule finding.
* 11.1-11.4 and 11.6 look at *explicit* casts only. 11.5, 11.8 and 11.9 also
  look at implicit conversions, so `int *p = malloc(n);` (void pointer to object
  pointer) is reported by 11.5 even though no cast is written.
* A null pointer constant (`0`, `(void*)0`, `NULL`) is exempt from 11.1-11.6
  and is judged by 11.9 instead. 11.9 accepts a null pointer conversion only
  when its source is spelled by the `NULL` macro; `p = 0`, `p == 0` and
  `(int*)0` are reported.
* 11.2 tolerates conversions between a pointer to an incomplete type and a
  `void` pointer (an interpretation; revisit against the licensed text).
* 11.3 treats every pair of differently typed complete object pointers alike,
  including casts to `char *`; qualifiers are ignored for the comparison.
* 11.4 and 11.6 do not report conversions to or from `_Bool`.
* 11.8 compares only the first pointer level; `char **` to `const char *const *`
  is not examined.

## Section 17 rules (functions) — 17.1-17.8
* **17.1** reports the `#include <stdarg.h>` directive and each `va_arg`
  expression (keyed on the invocation, see the macro policy).
* **17.2** builds a call graph and reports each call that lies on a cycle (a
  function calling itself, or mutual recursion through any number of functions).
  Static functions of the same name in different files are kept distinct.
  Cycles made of direct calls are reported as definite. Recursion that needs a
  function pointer is reported as **possible**, using two over-approximations:
  an indirect call may reach any project function whose address is taken and
  whose canonical function type is identical, and a function address passed to
  a library function (for example the comparator given to `qsort`) is assumed
  to be called back by it. Not followed: pointers cast to a different function
  type, addresses laundered through `void *` or an integer, and callbacks that
  a library function stores and invokes later. Where many callbacks share one
  signature the over-approximation can join unrelated functions into a cycle;
  that is why those findings are only possible. The catalogue lists this rule as
  undecidable.
* **17.3** and **17.4** (bare `return;`) concern constructs that recent Clang
  rejects as errors in C99 and later. The frontend downgrades a fixed list of
  such diagnostics to warnings so the file is still analysed and the rule can
  report it: `implicit-function-declaration`, `implicit-int`, `return-type`,
  `int-conversion`, `incompatible-pointer-types` and
  `incompatible-function-pointer-types`. Any other compile error still fails the
  translation unit (exit code 2). The downgrade is silent; it is applied after
  the project's own flags, so it overrides a project-wide `-Werror`. Compiler
  builtins (`__builtin_*`) are declared implicitly by design and are not
  reported by 17.3.
* **17.4** reports a `return;` in a non-void function, and a non-void function
  whose closing brace is reachable. Reachability uses Clang's control-flow
  graph (constant loop conditions and non-returning calls are understood), the
  same analysis behind the compiler's missing-return warning. `main` is
  included: the C99 implicit `return 0` does not exempt it.
* **17.5** compares the declared bound of an array parameter with an argument
  that is itself an array of known size. A pointer argument cannot be judged
  and is never reported. The catalogue lists this rule as undecidable.
* **17.6** reports `static` inside an array parameter declarator.
* **17.7** covers every call, including calls through function pointers.
  `(void)f();` is a deliberate discard and is accepted.
* **17.8** reports assignment, increment and decrement of the parameter itself.
  Modification through a pointer, or by taking the parameter's address, is not
  followed (the catalogue lists this rule as undecidable).

## Section 16 rules (switch) — 16.2, 16.3, 16.5, 16.6, 16.7 (16.4 earlier)
* **16.1 is not implemented.** Its well-formedness conditions are a list of
  syntactic requirements that overlap 16.2-16.6; reconstructing the exact list
  from memory risks a checker that disagrees with the licensed text, so it is
  left unimplemented until it can be checked against that text.
* A *clause* is one or more consecutive labels plus the statements up to the
  next label; chained labels (`case 1: case 2:`) are one clause.
* **16.3** accepts only `break` as a terminator, either as the last statement
  of the clause or as the last statement of a block that is the last statement.
  A clause ending in `return`, `goto`, `continue` or a call to a function that
  does not return is reported, and so is a fall-through marked by a comment.
  The final clause must also end with `break`.
* **16.2** judges the nearest enclosing compound statement. A switch whose body
  is not braced is left to 15.6.
* **16.5** compares label positions, not clause positions: `case 1: default:` is
  a default in last position.
* **16.6** counts clauses, so `case 1: case 2: ...; break;` is one clause.
* **16.7** uses the essential-type model, so a comparison, a `_Bool` and
  `<stdbool.h>` constants are Boolean.

## Section 12 rules (expressions) — 12.2, 12.3, 12.4
* **12.1 is not implemented.** Its list of operator pairs that need no
  parentheses could not be reconstructed with confidence; a wrong exemption list
  would either miss violations or flood results.
* This catalogue has no Rule 12.5 (it comes from a later MISRA amendment).
* **12.2** judges only *constant* shift counts: a count that is negative or not
  smaller than the width of the left operand's essential type is reported
  (`uint8_t << 8` counts as out of range even though promotion makes it legal C).
  A count held in a variable is not judged; the catalogue lists the rule as
  undecidable.
* **12.4** reports constant `+`, `-` and `*` of unsigned type whose result
  wraps, and unary minus of a non-zero unsigned constant (`-1u`). Left shifts
  that discard bits and conversions such as `(unsigned)-1` are not reported.
  Only expressions whose operands are integer constant expressions are judged,
  so a wrap that depends on a `const` variable is not seen.

## Section 9 rules (initialization) — 9.1-9.5
* **9.1** is undecidable; this is a source-order approximation and every
  finding is **possible**. An automatic scalar with no initializer is reported
  at its first read when no assignment, compound assignment, increment or
  address-of (`&x`, which is assumed to initialize it) occurs earlier in source
  order. A write counts from the *end* of its expression, so `x = x + 1` is a
  read before a write. Not modelled: branches and loops, so `if (c) x = 1;
  use(x);` is accepted (a miss) and a read that precedes the assignment only in
  source order inside a loop is reported (a false positive). Arrays, structures
  and `volatile` objects are not tracked, and `sizeof x` is not a read.
* **9.2** reports a nested aggregate initializer that has no braces of its own
  (brace elision), detected from the source character at the start of the nested
  list. The initializer `{0}` (possibly nested) is exempt for the whole object.
  Macro-expanded initializers are judged by the character at their spelling.
* **9.3** reports a constant-size array with fewer initializers than elements.
  Exempt: `{0}`, a list that uses designators, a string literal, and an array
  whose size comes from its initializer. A nested array is exempt only when its
  own list is `{0}`; whether a nested explicit list was written with designators
  is known only when Clang kept its original form.
* **9.4** reports the same designator path used twice in one list (`[0]` twice,
  `.x` twice). Overlapping but different paths (`.a = {...}, .a.b = 1`) and a
  designator that overrides a positional initializer are not detected.
* **9.5** reports a variable declared as an array without a size whose
  initializer uses array designators.

## Rules 8.1 and 8.13
* This catalogue has no Rules 8.15-8.17 (alignment specifiers come from a later
  MISRA amendment), so they are not registered.
* **8.1** (implicit `int`) is read from the compiler's own "type specifier
  missing" and "parameter was not declared" diagnostics, because the AST no
  longer records that the type was omitted. It therefore depends on the
  compiler reporting them: the frontend re-enables `-Wimplicit-int` after the
  project's flags, but `-w` (suppress all warnings) still hides them. The
  compiler's diagnostics are printed as before.
* **8.13** looks at pointer *parameters* only (not locals or members). A
  parameter is reported when every use in the function body only reads the
  pointee: dereference, subscript and member reads, comparisons, pointer
  arithmetic, and passing to a parameter that is pointer-to-const. Any use it
  does not recognise (passing to a non-const parameter, storing in a non-const
  pointer, taking the address, a cast) counts as a possible change and
  suppresses the finding, so the rule errs towards silence. Pointers to
  pointers, `void *`, function pointers and incomplete types are skipped, as are
  parameters that are never used. A function whose address is taken in the same
  translation unit is skipped (a callback type may fix its signature). A
  function with internal linkage is reported as definite; an externally
  visible one is **possible** because another file may bind it to a function
  pointer type. The catalogue lists the rule as undecidable.

## Section 20 rules (preprocessor) — 20.1-20.5, 20.7, 20.9-20.13
Text-level rules are checked on raw tokens of every project file (system
headers are skipped); macro rules use the macro definition as the preprocessor
saw it.
* **Not implemented:** 20.6 (it needs to know which macro arguments span lines
  and where a `#` begins a line inside them), 20.8 (it needs the *value* of each
  `#if` expression, which the preprocessor callbacks do not expose) and 20.14
  (a conditional directive that closes in another file is a hard compile error
  in Clang, so a detector could never fire; such a file fails analysis).
* **20.1** counts any token outside a directive as code, including tokens inside
  a group skipped by `#if 0`, so an include that follows a disabled block of
  code is reported.
* **20.2** looks at the header name as written; `//` and `/*` inside an
  angle-bracket name are reported even though the compiler may treat them as
  comments in other contexts.
* **20.3** reports an `#include` whose operand is neither `<...>` nor `"..."`.
* **20.4** uses the compiler's own keyword table for the language mode, so
  `inline`, `restrict` and `_Bool` are keywords in C99 but `bool` is not.
* **20.7** treats a parameter use as parenthesized when it sits between `(` and
  `)`, and also accepts a bare function-argument position (between `(` or `,`
  and `)` or `,`), which is an interpretation to confirm against the licensed
  text. Operands of `#` and `##` are exempt. `__VA_ARGS__` counts as a parameter.
* **20.9** reports an identifier that is not a macro at that point and is not
  the operand of `defined`; `__has_*` operators end the check for that
  expression. Only groups the preprocessor evaluates are checked.
* **20.10-20.12** work on the replacement list. `#` is an operator only in a
  function-like macro; `##` always is.
* **20.13** accepts the null directive and line markers, and checks skipped
  groups too (the raw scan does not skip them).

## Section 18 rules (pointers) — 18.1-18.7 (18.8 earlier)
18.4, 18.5 and 18.7 are decidable and implemented as written. 18.1, 18.2, 18.3
and 18.6 are undecidable in general; only the **provable subset** is checked,
so a clean result says nothing about code outside that subset. Everything
reported is definite.
* **18.1** reports a constant subscript outside a known array, and array `+`/`-`
  a constant that leaves `[0, size]` (one past the end is allowed, including
  `&a[size]`). A pointer whose origin is not a visible array, and any
  non-constant index, is not judged.
* **18.2** and **18.3** report subtraction and `<`, `<=`, `>`, `>=` between
  two pointers that certainly point into two *different named arrays* (a plain
  array, `&array[i]`, or array `+`/`-` n). Pointers held in variables are not
  traced, so `p - q` is never reported; `==` and `!=` are not covered by 18.3.
* **18.4** reports `+`, `-`, `+=` and `-=` when either operand is a pointer
  (including pointer difference). `++`, `--` and subscripting are not reported,
  matching the rule's operator list.
* **18.5** counts `*` levels (and array levels in between) on the type as
  written, so a typedef-name ends the count: `int_ptr **y` is two levels even
  though `int_ptr` hides one more. Parameters use the declared type before array
  adjustment, so `int *a[]` is one level.
* **18.6** reports `return &local;`, returning an automatic array (it decays to
  its address), and assigning such an address to a static or global object or
  through a pointer parameter (`*out = &local;`, `out->p = &local;`). Copies
  through intermediate pointers and aliases are not followed.
* **18.7** reports an array of unspecified size as the last member of a
  structure.

## Stage D rules — 3.1, 3.2, 4.1, 4.2, 6.1, 6.2, 7.2, 7.4, 13.1, 13.2, 13.3, 13.5, 13.6, 14.1, 14.2, 14.3, 15.3, 19.2
* **3.1/3.2** read comments with Clang's raw lexer. 3.1 reports a `/*` or `//`
  inside any comment (so a URL in a block comment is reported). 3.2 reports a
  `//` comment that is continued by a backslash-newline.
* **4.1** follows the strict reading: an octal or hexadecimal escape must be
  followed by the end of the literal or by another escape. `"\x41g"` and
  `"\0012"` are reported even though `g` is not a hex digit. Concatenated string
  tokens are judged one at a time, so `"\x41" "g"` is accepted.
* **4.2** finds `??` followed by a trigraph character anywhere in the text of a
  project file, including strings and comments.
* **6.1** accepts `_Bool`, `int` and `unsigned int`. Plain `int` cannot be told
  from `signed int` in the AST, so it is accepted (a leniency). **6.2** reports
  a *named* one-bit bit-field of signed type.
* **7.2** reports an integer constant of unsigned type written without `u`/`U`
  (typically a hexadecimal constant that does not fit `int`).
* **7.4** looks at initialization, assignment and `return` of a string literal
  to a pointer to non-const `char`. Initializer lists (`char *names[] = {"a"}`)
  and function arguments are not examined.
* **13.1/13.5/13.6** treat assignment, increment/decrement and `volatile` reads
  as definite side effects and any call as a *potential* one, reported as
  **possible** because the callee may be pure.
* **13.2** compares the operands whose evaluation order is unspecified
  (arithmetic, comparison and bitwise operators, call arguments, array subscript
  and its index, and the two sides of an assignment) for a scalar variable that
  one writes and another reads or writes. Members (`s.a`), pointer targets and
  effects of called functions are not tracked, so it reports only conflicts on
  named scalar variables.
* **13.3** treats the initializer of a declaration as an assignment and a call
  as another side effect; two increments alone are not reported.
* **14.1** looks at variables modified by a `for` increment clause. **14.2**
  reports an increment clause that modifies more than one variable or calls a
  function, a condition that assigns, increments or decrements, or a counter
  modified (or address-taken) in the body. Missing clauses (`for (;;)`) are not
  reported, which is an interpretation to confirm.
* **14.3** reports a condition with a value known at compile time, including
  `do { } while (0)` and `sizeof` comparisons; `while (1)` and `for (;;)` are
  accepted as the infinite-loop idiom (an interpretation).
* **15.3** requires the label's block to enclose the `goto`.
* **19.2** reports every union definition.

## Stage E rules — 21.1, 21.2, 21.12, 22.1-22.6
* **21.1** reports defining or undefining a macro whose name is reserved
  (leading double underscore, or underscore and a capital) or that the
  compiler or a system header already defines (the previous definition of the
  name is inspected). It does not report a `#define` of a standard *function*
  name that is not a macro (for example `#define printf my_printf`).
* **21.2** reports a declaration of a reserved identifier (leading double
  underscore or underscore-capital at any scope; any leading underscore at file
  scope; struct members excluded), and the *definition* of a function that the
  standard library declares (a prior declaration in a system header, or a
  compiler library builtin). A project that declares a standard name without
  the header and without a builtin is not noticed.
* **21.12** reports calls to the five `<fenv.h>` exception functions; the `FE_*`
  macros are not reported.
* The Section 22 rules are undecidable; each is a provable or source-order
  subset, and every finding is **possible** except 22.2 and 22.5:
  * **22.1** tracks a local pointer that receives an allocation (`malloc`,
    `calloc`, `realloc`, `aligned_alloc`, `strdup`, `strndup`, `fopen`,
    `freopen`, `tmpfile`) and reports it when no use releases or hands it on:
    reading and writing through it, comparing it and passing it to a
    pointer-to-const parameter are accepted as "still held". Passing it to any
    other function, returning it, storing it elsewhere or taking its address
    suppresses the finding. Allocations stored directly in members, globals or
    returned unassigned are not tracked.
  * **22.2** reports `free` applied to `&object`, an array, or a string literal.
  * **22.3** reports the same literal path opened twice in one function with at
    least one write-capable mode (`w`, `a`, `+`).
  * **22.4** reports a write call (`fprintf`, `vfprintf`, `fputs`, `fputc`,
    `putc`, `fwrite`) on a stream variable every `fopen` of which used a
    read-only literal mode.
  * **22.5** reports dereferencing a `FILE *`, member access through one, and
    declaring an object of type `FILE`.
  * **22.6** reports a stream variable used after `fclose` in source order,
    before any reassignment. Branches and loops are not modelled.

## Stage F rules — 1.1, 1.2, 1.3, 2.1, 2.2, 19.1
* **1.1** reports every compile *error* in a project file and the constraint
  violations the frontend deliberately downgrades (implicit function
  declaration, implicit int, a `return` without a value in a non-void function,
  a value returned from a void function). When a translation unit does not
  compile, its Rule 1.1 findings are still printed before the analysis stops
  with exit code 2; no other rule is evaluated for it. It does not claim to
  detect every syntax or constraint violation, only those the compiler reports.
* **1.2** reports every language extension the compiler diagnoses with
  `-pedantic` in a project file (statement expressions, binary literals, empty
  initializers and so on). Extensions the compiler accepts silently, such as
  `__attribute__` or `__typeof__`, are not reported. The frontend adds
  `-pedantic` itself, so an unrelated diagnostic may also appear on stderr.
* **1.3** is undecidable; the provable subset is integer division or remainder
  by a constant zero and signed integer overflow of a constant expression. Other
  undefined behaviour is reported by the rule that owns it (12.2 shifts, 13.2
  unsequenced access, 18.1 bounds, 17.4 missing return).
* **2.1** reports one finding per block that Clang's control-flow graph cannot
  reach: code after `return`, a branch of a condition that is a compile-time
  constant, and a lone `break` after `return`. Reachability is structural, so
  `if (DEBUG) { ... }` with a constant `DEBUG` is reported.
* **2.2** reports an expression statement whose value is computed and unused,
  using the compiler's own unused-result analysis; a call, an assignment, an
  increment and a cast to `void` are not reported. Stores that are never read
  (dead stores) are not detected.
* **19.1** reports struct self-assignment, assignment between members of one
  union object, and `memcpy`, `strcpy`, `strncpy`, `strcat` or `strncat` whose
  destination and source are the same array or the same pointer variable.
  `memmove` is allowed to overlap and is not reported. Overlap between two
  pointers that merely alias is not detected.

## Rules deliberately not implemented
Seven rules remain scaffolds that return `NotImplemented`. Each is left out for
a stated reason rather than approximated badly:
* **10.5** — the table of inappropriate casts between essential type
  categories could not be reconstructed with confidence.
* **11.7** — a cast between a pointer and a floating type is a constraint
  violation Clang rejects before analysis, so a detector could never fire.
* **12.1** — the list of operator pairs that need no parentheses could not be
  reconstructed with confidence.
* **16.1** — the well-formedness conditions of a switch statement overlap
  16.2-16.6 and could not be reconstructed with confidence.
* **20.6** — needs to know which macro arguments span lines and where a `#`
  begins a line inside them.
* **20.8** — needs the *value* of each `#if` expression, which the
  preprocessor callbacks do not expose.
* **20.14** — a conditional directive that closes in another file is a hard
  compile error in Clang, so a detector could never fire.

Every implemented rule still reads "independent validation pending" and every
interpretation above must be confirmed against the licensed MISRA text before
any compliance claim is made.
