# Rule 22.5

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Reports a dereference of a FILE pointer or member access through it.

Detection contract: Report dereference of a FILE pointer or member access through it.

## Known limits

Passing the pointer to library functions is the intended use and is accepted.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | only passing the pointer | pass |
| Violation (must be reported) | dereferencing the pointer to copy the object | pass |
| Compliant (must stay silent) | sizeof of the dereferenced pointer is not evaluated | pass |
| Violation (must be reported) | member access through a stream pointer | pass |

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

### Compliant (must stay silent): sizeof of the dereferenced pointer is not evaluated

`main.c`

```c
#include <stdio.h>
unsigned long f(FILE *p) { return sizeof(*p); }
```

### Violation (must be reported): member access through a stream pointer

`main.c`

```c
#include <stdio.h>
int f(FILE *p) { return p->_flags; } /*@V*/
```

