# Rule 22.4

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Subset: reports a write to a stream that was opened only for reading in the same function.

Detection contract: Subset: report a write to a stream opened only for reading in the same function.

## Known limits

Only streams opened with a literal mode in the same function are tracked.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | writing to a stream opened for writing | pass |
| Violation (must be reported) | writing to a read-only stream | pass |
| Compliant (must stay silent) | update mode allows writing | pass |
| Violation (must be reported) | fprintf to a binary read stream | pass |
| Compliant (must stay silent) | a stream parameter of unknown mode | pass |
| Violation (must be reported) | fputc to a read-only stream | pass |
| Violation (must be reported) | putc to a read-only stream | pass |
| Compliant (must stay silent) | writing to a stream opened for writing | pass |
| Compliant (must stay silent) | writing to a stream opened for appending | pass |

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

### Compliant (must stay silent): a stream parameter of unknown mode

`main.c`

```c
#include <stdio.h>
void f(FILE *s) { (void)fputs("a", s); }
```

### Violation (must be reported): fputc to a read-only stream

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "r");
    (void)fputc('a', a); /*@V*/
}
```

### Violation (must be reported): putc to a read-only stream

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "r");
    (void)putc('a', a); /*@V*/
}
```

### Compliant (must stay silent): writing to a stream opened for writing

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "w");
    (void)fputc('a', a);
}
```

### Compliant (must stay silent): writing to a stream opened for appending

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "a");
    (void)fputc('a', a);
}
```

