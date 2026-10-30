# Rule 11.8

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports an explicit pointer cast that removes const or volatile from the pointed-to type.

Detection contract: Report an explicit pointer cast that drops const or volatile from the pointee type.

## Known limits

Only explicit casts are examined; an implicit conversion that drops a qualifier is a compiler diagnostic.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | adding a qualifier | pass |
| Compliant (must stay silent) | casting without changing qualifiers | pass |
| Violation (must be reported) | casting away const | pass |
| Violation (must be reported) | casting away volatile | pass |
| Violation (must be reported) | casting away const from a const void pointer | pass |
| Compliant (must stay silent) | casting to a pointer with more qualifiers | pass |
| Violation (must be reported) | const removed by a cast | pass |
| Violation (must be reported) | volatile removed by a cast | pass |
| Compliant (must stay silent) | const kept by the cast | pass |

## Case sources

### Compliant (must stay silent): adding a qualifier

`main.c`

```c
const int *f(int *p) { return (const int *)p; }
```

### Compliant (must stay silent): casting without changing qualifiers

`main.c`

```c
int *g(int *p) { return (int *)p; }
```

### Violation (must be reported): casting away const

`main.c`

```c
int *f(const int *p) { return (int *)p; } /*@V*/
```

### Violation (must be reported): casting away volatile

`main.c`

```c
int *g(volatile int *p) { return (int *)p; } /*@V*/
```

### Violation (must be reported): casting away const from a const void pointer

`main.c`

```c
void f(const void *v) { int *p = (int *)v; (void)p; } /*@V*/
```

### Compliant (must stay silent): casting to a pointer with more qualifiers

`main.c`

```c
const volatile int *f(int *p) { return (const volatile int *)p; }
```

### Violation (must be reported): const removed by a cast

`main.c`

```c
void f(const int *p)
{
    int *q = (int *)p; /*@V*/
    (void)q;
}
```

### Violation (must be reported): volatile removed by a cast

`main.c`

```c
void f(volatile int *p)
{
    int *q = (int *)p; /*@V*/
    (void)q;
}
```

### Compliant (must stay silent): const kept by the cast

`main.c`

```c
const int *f(const int *p) { return (const int *)p; }
```

