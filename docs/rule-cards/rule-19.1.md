# Rule 19.1

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Overlapping storage

## What the detector reports

Subset: reports self-assignment and copy calls whose source and destination are the same expression.

Detection contract: Subset: report self-assignment and copy calls whose source and destination are the same expression.

## Known limits

Only syntactically identical operands are compared; overlapping objects reached by different expressions are not found.

- **False negative:** overlap through different expressions (documented limit)

## Tested cases

8 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | assignment between different objects | pass |
| Compliant (must stay silent) | a copy between different buffers | pass |
| Violation (must be reported) | self-assignment | pass |
| Violation (must be reported) | copying a buffer onto itself | pass |
| Known false negative (not detected yet) | overlap through different expressions | documented limit |
| Violation (must be reported) | array element assigned to itself | pass |
| Violation (must be reported) | dereference assigned to itself | pass |
| Compliant (must stay silent) | different elements | pass |
| Violation (must be reported) | memcpy of a buffer onto itself through member | pass |

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

### Violation (must be reported): array element assigned to itself

`main.c`

```c
void f(int *a, int i) { a[i] = a[i]; } /*@V*/
```

### Violation (must be reported): dereference assigned to itself

`main.c`

```c
void f(int *p) { *p = *p; } /*@V*/
```

### Compliant (must stay silent): different elements

`main.c`

```c
void f(int *a) { a[0] = a[1]; }
```

### Violation (must be reported): memcpy of a buffer onto itself through member

`main.c`

```c
#include <string.h>
struct s { char b[8]; };
void f(struct s *p) { memcpy(p->b, p->b, 8); } /*@V*/
```

