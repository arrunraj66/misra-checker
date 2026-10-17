# Rule 1.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard C conformance

## What the detector reports

Reports every compiler error (a syntax error or a constraint violation, including an include file that cannot be found) located in the main file.

Detection contract: Report every compiler error (syntax or constraint violation) located in the main file.

## Known limits

After a missing header Clang may stop reading later includes, so code that depends on them can be reported as errors too (compile errors hide types and declarations).

## Tested cases

6 of 6 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | valid code, including constructs that only warn | pass |
| Violation (must be reported) | syntax error | pass |
| Violation (must be reported) | use of an undeclared identifier | pass |
| Violation (must be reported) | constraint violation (assigning a structure to an int) | pass |
| Violation (must be reported) | include file that does not exist | pass |
| Violation (must be reported) | an error located in a project header is reported at the header | pass |

## Case sources

### Compliant (must stay silent): valid code, including constructs that only warn

`main.c`

```c
int f(int unused_param) { int never_used; return 1; }
```

### Violation (must be reported): syntax error

`main.c`

```c
int x = ; /*@V*/
```

### Violation (must be reported): use of an undeclared identifier

`main.c`

```c
int f(void) { return missing_name; } /*@V*/
```

### Violation (must be reported): constraint violation (assigning a structure to an int)

`main.c`

```c
struct S { int a; };
int g(struct S s) { int i = s; return i; } /*@V*/
```

### Violation (must be reported): include file that does not exist

`main.c`

```c
#include "no_such_header_anywhere.h" /*@V*/
int a;
```

### Violation (must be reported): an error located in a project header is reported at the header

`main.c`

```c
#include "bad.h"
int a;
```

`bad.h`

```c
int x = ; /*@V*/
```

