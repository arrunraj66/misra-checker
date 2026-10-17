# Rule 8.6

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Declarations and definitions

## What the detector reports

Whole-program check: reports an external name defined in more than one translation unit, or referenced but never defined.

Detection contract: Whole-program: report an external name defined in more than one translation unit, or referenced but never defined (requires every translation unit of the program in one run; Inconclusive otherwise).

## Known limits

Needs every translation unit of the program in one run (library functions and compiler built-ins are not reported as undefined).

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | defined once, used elsewhere | pass |
| Violation (must be reported) | the same function defined in two files | pass |
| Violation (must be reported) | a function that is referenced but defined nowhere | pass |
| Compliant (must stay silent) | standard library functions are not reported | pass |

## Case sources

### Compliant (must stay silent): defined once, used elsewhere

`a.c`

```c
int value(void) { return 1; }
```

`b.c`

```c
int value(void);
int use(void) { return value(); }
```

### Violation (must be reported): the same function defined in two files

`a.c`

```c
int dup(void) { return 1; } /*@V*/
```

`b.c`

```c
int dup(void) { return 2; } /*@V*/
```

### Violation (must be reported): a function that is referenced but defined nowhere

`a.c`

```c
int missing(void);
int use(void) { return missing(); } /*@V*/
```

`b.c`

```c
int top(void);
int main(void) { return top(); }
```

### Compliant (must stay silent): standard library functions are not reported

`main.c`

```c
#include <string.h>
unsigned long n(const char *s) { return strlen(s); }
```

