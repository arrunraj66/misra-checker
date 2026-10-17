# Rule 11.9

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports a null pointer constant that is not written with the NULL macro (for example a plain 0 or (void *)0).

Detection contract: Report a null pointer constant that is not spelled through the NULL macro.

## Known limits

NULL passed through the argument of another macro still counts as NULL.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | NULL and comparisons without a constant | pass |
| Compliant (must stay silent) | a pointer used as a condition | pass |
| Violation (must be reported) | zero as a null pointer | pass |
| Violation (must be reported) | comparison with zero | pass |
| Violation (must be reported) | a cast zero | pass |
| Violation (must be reported) | zero passed where a pointer is expected | pass |
| Violation (must be reported) | a home made null macro | pass |
| Compliant (must stay silent) | NULL passed as an argument | pass |

## Case sources

### Compliant (must stay silent): NULL and comparisons without a constant

`main.c`

```c
#include <stddef.h>
int *p = NULL;
int f(int *q) { return q != NULL && *q > 0; }
```

### Compliant (must stay silent): a pointer used as a condition

`main.c`

```c
int g(int *q) { if (q) { return 1; } return 0; }
```

### Violation (must be reported): zero as a null pointer

`main.c`

```c
int *p = 0; /*@V*/
```

### Violation (must be reported): comparison with zero

`main.c`

```c
int f(int *p) { return p == 0; } /*@V*/
```

### Violation (must be reported): a cast zero

`main.c`

```c
void *q = (void *)0; /*@V*/
```

### Violation (must be reported): zero passed where a pointer is expected

`main.c`

```c
void take(int *p);
void f(void) { take(0); } /*@V*/
```

### Violation (must be reported): a home made null macro

`main.c`

```c
#define MYNULL ((void *)0)
int *p = MYNULL; /*@V*/
```

### Compliant (must stay silent): NULL passed as an argument

`main.c`

```c
#include <stddef.h>
void take(int *p);
void f(void) { take(NULL); }
```

