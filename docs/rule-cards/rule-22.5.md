# Rule 22.5

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Reports a dereference of a FILE pointer or member access through it.

Detection contract: Report dereference of a FILE pointer or member access through it.

## Known limits

Passing the pointer to library functions is the intended use and is accepted.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | only passing the pointer | pass |
| Violation (must be reported) | dereferencing the pointer to copy the object | pass |

## Case sources

### Compliant (must stay silent): only passing the pointer

`main.c`

```c
#include <stdio.h>
void f(FILE *s) { (void)fputc('a', s); }
```

### Violation (must be reported): dereferencing the pointer to copy the object

`main.c`

```c
#include <stdio.h>
void f(FILE *s)
{
    FILE copy = *s; /*@V*/
    (void)copy;
}
```

