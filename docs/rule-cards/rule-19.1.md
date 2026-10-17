# Rule 19.1

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Overlapping storage

## What the detector reports

Subset: reports self-assignment and copy calls whose source and destination are the same expression.

Detection contract: Subset: report self-assignment and copy calls whose source and destination are the same expression.

## Known limits

Only syntactically identical operands are compared; overlapping objects reached by different expressions are not found.

- **False negative:** overlap through different expressions (documented limit)

## Tested cases

4 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | assignment between different objects | pass |
| Compliant (must stay silent) | a copy between different buffers | pass |
| Violation (must be reported) | self-assignment | pass |
| Violation (must be reported) | copying a buffer onto itself | pass |
| Known false negative (not detected yet) | overlap through different expressions | documented limit |

## Case sources

### Compliant (must stay silent): assignment between different objects

`main.c`

```c
void f(int a, int b) { a = b; (void)a; }
```

### Compliant (must stay silent): a copy between different buffers

`main.c`

```c
#include <string.h>
void g(char *d, const char *s) { memcpy(d, s, 4); }
```

### Violation (must be reported): self-assignment

`main.c`

```c
void f(int a)
{
    a = a; /*@V*/
}
```

### Violation (must be reported): copying a buffer onto itself

`main.c`

```c
#include <string.h>
void g(char *p) { memcpy(p, p, 4); } /*@V*/
```

### Known false negative (not detected yet): overlap through different expressions

`main.c`

```c
#include <string.h>
void h(char *p) { memcpy(p, p + 1, 4); } /*@V*/
```

