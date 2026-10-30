# Rule 22.6

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Subset: reports a stream used after fclose later in the same block without reassignment.

Detection contract: Subset: report a stream used after fclose later in the same block without reassignment.

## Known limits

Only straight-line uses in the same block are found.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | reassigned after the close | pass |
| Violation (must be reported) | used after fclose | pass |
| Violation (must be reported) | the pointer tested after fclose | pass |
| Compliant (must stay silent) | the pointer cleared after fclose | pass |
| Violation (must be reported) | fprintf after fclose | pass |
| Compliant (must stay silent) | close in one branch and use in the other | pass |
| Violation (must be reported) | reading a stream after it is closed | pass |
| Compliant (must stay silent) | reopening into a new variable | pass |

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

### Violation (must be reported): the pointer tested after fclose

`main.c`

```c
#include <stdio.h>
int f(FILE *p)
{
    (void)fclose(p);
    if (p != NULL) { return 1; } /*@V*/
    return 0;
}
```

### Compliant (must stay silent): the pointer cleared after fclose

`main.c`

```c
#include <stdio.h>
void f(FILE *p)
{
    (void)fclose(p);
    p = NULL;
}
```

### Violation (must be reported): fprintf after fclose

`main.c`

```c
#include <stdio.h>
void f(FILE *p)
{
    (void)fclose(p);
    (void)fprintf(p, "x"); /*@V*/
}
```

### Compliant (must stay silent): close in one branch and use in the other

`main.c`

```c
#include <stdio.h>
void f(FILE *p, int c)
{
    if (c) { (void)fclose(p); } else { (void)fputs("x", p); }
}
```

### Violation (must be reported): reading a stream after it is closed

`main.c`

```c
#include <stdio.h>
int f(FILE *p)
{
    (void)fclose(p);
    return fgetc(p); /*@V*/
}
```

### Compliant (must stay silent): reopening into a new variable

`main.c`

```c
#include <stdio.h>
int f(FILE *p)
{
    (void)fclose(p);
    FILE *q = fopen("x", "r");
    return (q != (FILE *)0) ? fgetc(q) : -1;
}
```

