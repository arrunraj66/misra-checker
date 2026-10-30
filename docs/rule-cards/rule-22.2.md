# Rule 22.2

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Resource management

## What the detector reports

Subset: reports freeing the address of an object, an array or a string literal, and a straight-line double free.

Detection contract: Subset: report freeing the address of an object, an array or a string literal, and straight-line double free.

## Known limits

Only the stated forms in one function are found; frees on different paths are not matched.

## Tested cases

10 of 10 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | one free of an allocation | pass |
| Violation (must be reported) | freeing the address of an object | pass |
| Violation (must be reported) | freeing an array | pass |
| Violation (must be reported) | freeing a string literal | pass |
| Violation (must be reported) | freeing twice in a row | pass |
| Violation (must be reported) | free of a structure member address | pass |
| Compliant (must stay silent) | free of an allocation obtained earlier | pass |
| Compliant (must stay silent) | free of a pointer parameter | pass |
| Compliant (must stay silent) | freeing a null pointer constant | pass |
| Compliant (must stay silent) | freeing after reassignment | pass |

## Case sources

### Compliant (must stay silent): one free of an allocation

`main.c`

```c
#include <stdlib.h>
void f(void) { char *p = malloc(4); free(p); }
```

### Violation (must be reported): freeing the address of an object

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    int x;
    free(&x); /*@V*/
}
```

### Violation (must be reported): freeing an array

`main.c`

```c
#include <stdlib.h>
void g(void)
{
    char buf[4];
    free(buf); /*@V*/
}
```

### Violation (must be reported): freeing a string literal

`main.c`

```c
#include <stdlib.h>
void h(void) { free("text"); } /*@V*/
```

### Violation (must be reported): freeing twice in a row

`main.c`

```c
#include <stdlib.h>
void k(void)
{
    char *p = malloc(4);
    free(p);
    free(p); /*@V*/
}
```

### Violation (must be reported): free of a structure member address

`main.c`

```c
#include <stdlib.h>
struct s { int a; };
void f(void)
{
    struct s v;
    free(&v.a); /*@V*/
}
```

### Compliant (must stay silent): free of an allocation obtained earlier

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    char *p = malloc(4);
    free(p);
}
```

### Compliant (must stay silent): free of a pointer parameter

`main.c`

```c
#include <stdlib.h>
void f(void *p) { free(p); }
```

### Compliant (must stay silent): freeing a null pointer constant

`main.c`

```c
#include <stdlib.h>
void f(void) { free(0); }
```

### Compliant (must stay silent): freeing after reassignment

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    char *p = malloc(4);
    free(p);
    p = malloc(4);
    free(p);
}
```

