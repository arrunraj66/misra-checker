# Rule 22.4

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Subset: reports a write to a stream that was opened only for reading in the same function.

Detection contract: Subset: report a write to a stream opened only for reading in the same function.

## Known limits

Only streams opened with a literal mode in the same function are tracked.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | writing to a stream opened for writing | pass |
| Violation (must be reported) | writing to a read-only stream | pass |
| Compliant (must stay silent) | update mode allows writing | pass |
| Violation (must be reported) | fprintf to a binary read stream | pass |

## Case sources

### Compliant (must stay silent): writing to a stream opened for writing

`main.c`

```c
#include <stdio.h>
void f(void) { FILE *o = fopen("a", "w"); if (o != 0) { (void)fputs("x", o); (void)fclose(o); } }
```

### Violation (must be reported): writing to a read-only stream

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *i = fopen("a", "r");
    if (i != 0)
    {
        (void)fputs("x", i); /*@V*/
        (void)fclose(i);
    }
}
```

### Compliant (must stay silent): update mode allows writing

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "r+");
    (void)fputs("a", a);
}
```

### Violation (must be reported): fprintf to a binary read stream

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "rb");
    (void)fprintf(a, "a"); /*@V*/
}
```

