# Rule 18.3

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Pointers and arrays

## What the detector reports

Subset: reports a relational comparison of pointers derived from two different declared arrays.

Detection contract: Subset: report relational comparison of pointers derived from two different declared arrays.

## Known limits

Only pointers formed directly from named arrays are examined.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | comparing within one array | pass |
| Violation (must be reported) | comparing across arrays | pass |
| Violation (must be reported) | with the other relational operators | pass |
| Compliant (must stay silent) | pointer parameters of unknown origin | pass |
| Violation (must be reported) | relational comparison of two distinct arrays through addresses of elements | pass |

## Case sources

### Compliant (must stay silent): comparing within one array

`main.c`

```c
int a[8];
int f(void) { return &a[1] < &a[5]; }
```

### Violation (must be reported): comparing across arrays

`main.c`

```c
int a[8];
int b[8];
int f(void) { return &a[1] < &b[1]; } /*@V*/
```

### Violation (must be reported): with the other relational operators

`main.c`

```c
int a[8];
int b[8];
int g(void) { return &a[0] >= &b[0]; } /*@V*/
```

### Compliant (must stay silent): pointer parameters of unknown origin

`main.c`

```c
int f(int *p, int *q) { return p < q; }
```

### Violation (must be reported): relational comparison of two distinct arrays through addresses of elements

`main.c`

```c
int a[4];
int b[4];
int f(void) { return &a[1] >= &b[1]; } /*@V*/
```

