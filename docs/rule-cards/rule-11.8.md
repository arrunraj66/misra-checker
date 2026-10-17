# Rule 11.8

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports an explicit pointer cast that removes const or volatile from the pointed-to type.

Detection contract: Report an explicit pointer cast that drops const or volatile from the pointee type.

## Known limits

Only explicit casts are examined; an implicit conversion that drops a qualifier is a compiler diagnostic.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | adding a qualifier | pass |
| Compliant (must stay silent) | casting without changing qualifiers | pass |
| Violation (must be reported) | casting away const | pass |
| Violation (must be reported) | casting away volatile | pass |

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

