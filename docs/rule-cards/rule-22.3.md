# Rule 22.3

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Subset: reports a file opened twice in one function with at least one of the two openings in a write mode.

Detection contract: Subset: report a file opened twice in one function with at least one write mode.

## Known limits

Only file names written as literals in one function are compared.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | the same file opened twice for reading | pass |
| Compliant (must stay silent) | different files | pass |
| Violation (must be reported) | one file opened for writing and for reading | pass |
| Compliant (must stay silent) | opened for reading with read plus mode twice | pass |
| Violation (must be reported) | append and read of the same name | pass |
| Violation (must be reported) | the same file for reading and updating | pass |
| Violation (must be reported) | the same file opened for reading and for writing | pass |
| Compliant (must stay silent) | two different files | pass |
| Compliant (must stay silent) | the same file opened twice for reading | pass |

## Case sources

### Compliant (must stay silent): the same file opened twice for reading

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("data", "r");
    FILE *b = fopen("data", "r");
    if (a != 0) { (void)fclose(a); }
    if (b != 0) { (void)fclose(b); }
}
```

### Compliant (must stay silent): different files

`main.c`

```c
#include <stdio.h>
void g(void)
{
    FILE *a = fopen("one", "w");
    FILE *b = fopen("two", "r");
    if (a != 0) { (void)fclose(a); }
    if (b != 0) { (void)fclose(b); }
}
```

### Violation (must be reported): one file opened for writing and for reading

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("data", "w");
    FILE *b = fopen("data", "r"); /*@V*/
    if (a != 0) { (void)fclose(a); }
    if (b != 0) { (void)fclose(b); }
}
```

### Compliant (must stay silent): opened for reading with read plus mode twice

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "rb");
    FILE *b = fopen("x", "rb");
    (void)a; (void)b;
}
```

### Violation (must be reported): append and read of the same name

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "a");
    FILE *b = fopen("x", "r"); /*@V*/
    (void)a; (void)b;
}
```

### Violation (must be reported): the same file for reading and updating

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("x", "r");
    FILE *b = fopen("x", "r+"); /*@V*/
    (void)a; (void)b;
}
```

### Violation (must be reported): the same file opened for reading and for writing

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("data.txt", "r");
    FILE *b = fopen("data.txt", "w"); /*@V*/
    (void)a;
    (void)b;
}
```

### Compliant (must stay silent): two different files

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("in.txt", "r");
    FILE *b = fopen("out.txt", "w");
    (void)a;
    (void)b;
}
```

### Compliant (must stay silent): the same file opened twice for reading

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *a = fopen("data.txt", "r");
    FILE *b = fopen("data.txt", "r");
    (void)a;
    (void)b;
}
```

