# Rule 21.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to malloc, calloc, realloc or free (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: dynamic memory allocation function called (matched by callee name).

## Known limits

Calls through function pointers and wrappers with other names are not found.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no dynamic allocation | pass |
| Compliant (must stay silent) | a function with a similar name | pass |
| Violation (must be reported) | malloc and free | pass |
| Violation (must be reported) | calloc and realloc | pass |

## Case sources

### Compliant (must stay silent): no dynamic allocation

`main.c`

```c
static char pool[64];
char *get(void) { return pool; }
```

### Compliant (must stay silent): a function with a similar name

`main.c`

```c
void *malloc_pool(unsigned long n);
void *use(void) { return malloc_pool(4); }
```

### Violation (must be reported): malloc and free

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    void *p = malloc(4); /*@V*/
    free(p); /*@V*/
}
```

### Violation (must be reported): calloc and realloc

`main.c`

```c
#include <stdlib.h>
void g(void)
{
    void *p = calloc(2, 4); /*@V*/
    p = realloc(p, 8); /*@V*/
    (void)p;
}
```

