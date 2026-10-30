# Rule 22.1

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Subset: reports a local object that holds an allocation or an open stream and is neither released nor handed on in the function.

Detection contract: Subset: report a local object holding an allocation or open stream that is neither released nor handed on in the function.

## Known limits

Only the owner within one function is tracked; passing the object to any function counts as handing it on.

## Tested cases

15 of 15 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | allocation that is freed | pass |
| Compliant (must stay silent) | ownership returned to the caller | pass |
| Compliant (must stay silent) | a stream that is closed | pass |
| Compliant (must stay silent) | handed on to another function | pass |
| Violation (must be reported) | allocation that is never released | pass |
| Violation (must be reported) | a stream that is never closed | pass |
| Compliant (must stay silent) | allocation checked for null then freed | pass |
| Compliant (must stay silent) | allocation stored in a global | pass |
| Violation (must be reported) | calloc never freed | pass |
| Compliant (must stay silent) | a stream returned to the caller | pass |
| Compliant (must stay silent) | the allocation is passed to a function that takes over | pass |
| Compliant (must stay silent) | an allocation freed after use on every path | pass |
| Compliant (must stay silent) | the allocation is stored in a structure that outlives the function | pass |
| Compliant (must stay silent) | realloc keeps the owner | pass |
| Violation (must be reported) | the owner is overwritten, losing the allocation | pass |

## Case sources

### Compliant (must stay silent): allocation that is freed

`main.c`

```c
#include <stdlib.h>
void f(void) { char *p = malloc(4); free(p); }
```

### Compliant (must stay silent): ownership returned to the caller

`main.c`

```c
#include <stdlib.h>
char *g(void) { char *p = malloc(4); return p; }
```

### Compliant (must stay silent): a stream that is closed

`main.c`

```c
#include <stdio.h>
void h(void) { FILE *f = fopen("a", "r"); if (f != 0) { (void)fclose(f); } }
```

### Compliant (must stay silent): handed on to another function

`main.c`

```c
#include <stdlib.h>
void keep(char *p);
void k(void) { char *p = malloc(4); keep(p); }
```

### Violation (must be reported): allocation that is never released

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    char *p = malloc(4); /*@V*/
    (void)p;
}
```

### Violation (must be reported): a stream that is never closed

`main.c`

```c
#include <stdio.h>
void g(void)
{
    FILE *f = fopen("a", "r"); /*@V*/
    (void)f;
}
```

### Compliant (must stay silent): allocation checked for null then freed

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    char *p = malloc(4);
    if (p != NULL)
    {
        p[0] = 'a';
        free(p);
    }
}
```

### Compliant (must stay silent): allocation stored in a global

`main.c`

```c
#include <stdlib.h>
char *keep;
void f(void) { keep = malloc(4); }
```

### Violation (must be reported): calloc never freed

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    char *p = calloc(1, 4); /*@V*/
    if (p != NULL) { p[0] = 'a'; }
}
```

### Compliant (must stay silent): a stream returned to the caller

`main.c`

```c
#include <stdio.h>
FILE *f(void) { return fopen("a", "r"); }
```

### Compliant (must stay silent): the allocation is passed to a function that takes over

`main.c`

```c
#include <stdlib.h>
void take(char *p);
void f(void)
{
    char *p = malloc(4);
    take(p);
}
```

### Compliant (must stay silent): an allocation freed after use on every path

`main.c`

```c
#include <stdlib.h>
int f(int n)
{
    char *p = malloc(8);
    int r = 0;
    if (p != 0)
    {
        p[0] = (char)n;
        r = p[0];
        free(p);
    }
    return r;
}
```

### Compliant (must stay silent): the allocation is stored in a structure that outlives the function

`main.c`

```c
#include <stdlib.h>
struct holder { char *buf; };
void f(struct holder *h) { h->buf = malloc(8); }
```

### Compliant (must stay silent): realloc keeps the owner

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    char *p = malloc(8);
    p = realloc(p, 16);
    free(p);
}
```

### Violation (must be reported): the owner is overwritten, losing the allocation

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    char *p = malloc(8); /*@V*/
    p = 0;
}
```

