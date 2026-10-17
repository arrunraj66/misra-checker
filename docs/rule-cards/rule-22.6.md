# Rule 22.6

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Subset: reports a stream used after fclose later in the same block without reassignment.

Detection contract: Subset: report a stream used after fclose later in the same block without reassignment.

## Known limits

Only straight-line uses in the same block are found.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | reassigned after the close | pass |
| Violation (must be reported) | used after fclose | pass |

## Case sources

### Compliant (must stay silent): reassigned after the close

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *s = fopen("a", "r");
    if (s != 0) { (void)fclose(s); }
    s = fopen("b", "r");
    if (s != 0) { (void)fclose(s); }
}
```

### Violation (must be reported): used after fclose

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *s = fopen("a", "w");
    if (s != 0)
    {
        (void)fclose(s);
        (void)fputc('x', s); /*@V*/
    }
}
```

