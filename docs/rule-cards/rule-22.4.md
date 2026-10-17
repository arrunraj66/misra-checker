# Rule 22.4

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Subset: reports a write to a stream that was opened only for reading in the same function.

Detection contract: Subset: report a write to a stream opened only for reading in the same function.

## Known limits

Only streams opened with a literal mode in the same function are tracked.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | writing to a stream opened for writing | pass |
| Violation (must be reported) | writing to a read-only stream | pass |

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

