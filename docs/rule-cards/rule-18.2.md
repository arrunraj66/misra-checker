# Rule 18.2

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Pointers and arrays

## What the detector reports

Subset: reports the subtraction of pointers derived from two different declared arrays.

Detection contract: Subset: report subtraction of pointers derived from two different declared arrays.

## Known limits

Only pointers formed directly from named arrays are examined.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | pointers into the same array | pass |
| Violation (must be reported) | pointers into two arrays | pass |
| Violation (must be reported) | array names of two arrays | pass |

## Case sources

### Compliant (must stay silent): pointers into the same array

`main.c`

```c
int a[8];
long d(void) { return &a[6] - &a[1]; }
```

### Violation (must be reported): pointers into two arrays

`main.c`

```c
int a[8];
int b[8];
long d(void) { return &a[1] - &b[0]; } /*@V*/
```

### Violation (must be reported): array names of two arrays

`main.c`

```c
int a[8];
int b[8];
long e(void) { return (a + 2) - b; } /*@V*/
```

