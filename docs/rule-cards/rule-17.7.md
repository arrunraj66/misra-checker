# Rule 17.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Functions

## What the detector reports

Reports a call to a non-void function used as an expression statement (a cast to void is accepted).

Detection contract: Report a call to a non-void function used as an expression statement (a cast to void is accepted).

## Known limits

Only calls whose result is discarded as a statement are examined.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | the result is used or cast to void | pass |
| Compliant (must stay silent) | a void function | pass |
| Violation (must be reported) | the result of a function is discarded | pass |
| Violation (must be reported) | the result of a library function is discarded | pass |
| Compliant (must stay silent) | a library call cast to void | pass |
| Compliant (must stay silent) | the call is used in a condition | pass |
| Violation (must be reported) | a call whose result is dropped inside a block | pass |

## Case sources

### Compliant (must stay silent): the result is used or cast to void

`main.c`

```c
int g(void);
void f(void) { int x = g(); (void)g(); (void)x; }
```

### Compliant (must stay silent): a void function

`main.c`

```c
void h(void);
void k(void) { h(); }
```

### Violation (must be reported): the result of a function is discarded

`main.c`

```c
int g(void);
void f(void)
{
    g(); /*@V*/
}
```

### Violation (must be reported): the result of a library function is discarded

`main.c`

```c
#include <string.h>
void f(char *d, const char *s)
{
    strcpy(d, s); /*@V*/
}
```

### Compliant (must stay silent): a library call cast to void

`main.c`

```c
#include <string.h>
void f(char *d, const char *s) { (void)strcpy(d, s); }
```

### Compliant (must stay silent): the call is used in a condition

`main.c`

```c
int g(void);
void f(void) { if (g() > 0) { } }
```

### Violation (must be reported): a call whose result is dropped inside a block

`main.c`

```c
int g(void);
void f(int c)
{
    if (c)
    {
        g(); /*@V*/
    }
}
```

