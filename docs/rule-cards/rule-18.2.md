# Rule 18.2

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Pointers and arrays

## What the detector reports

Subset: reports the subtraction of pointers derived from two different declared arrays.

Detection contract: Subset: report subtraction of pointers derived from two different declared arrays.

## Known limits

Only pointers formed directly from named arrays are examined.

- **False negative:** members that are different arrays (only declared array objects are followed) (documented limit)

## Tested cases

8 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | pointers into the same array | pass |
| Violation (must be reported) | pointers into two arrays | pass |
| Violation (must be reported) | array names of two arrays | pass |
| Known false negative (not detected yet) | members that are different arrays (only declared array objects are followed) | documented limit |
| Compliant (must stay silent) | pointer parameters of unknown origin | pass |
| Violation (must be reported) | subtracting pointers into two local arrays | pass |
| Compliant (must stay silent) | subtracting pointers into the same array | pass |
| Compliant (must stay silent) | subtracting through a pointer to the same array parameter | pass |
| Compliant (must stay silent) | a pointer that is advanced is not tied to its first array | pass |

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

### Known false negative (not detected yet): members that are different arrays (only declared array objects are followed)

`main.c`

```c
struct s { int a[4]; int b[4]; };
long f(struct s *p) { return &p->a[0] - &p->b[0]; } /*@V*/
```

### Compliant (must stay silent): pointer parameters of unknown origin

`main.c`

```c
long f(int *p, int *q) { return p - q; }
```

### Violation (must be reported): subtracting pointers into two local arrays

`main.c`

```c
int f(void)
{
    int a[4];
    int b[4];
    int *p = &a[1];
    int *q = &b[2];
    return (int)(q - p); /*@V*/
}
```

### Compliant (must stay silent): subtracting pointers into the same array

`main.c`

```c
int f(void)
{
    int a[8];
    int *p = &a[1];
    int *q = &a[6];
    return (int)(q - p);
}
```

### Compliant (must stay silent): subtracting through a pointer to the same array parameter

`main.c`

```c
int f(int *base, int *end) { return (int)(end - base); }
```

### Compliant (must stay silent): a pointer that is advanced is not tied to its first array

`main.c`

```c
int f(void)
{
    int a[4];
    int b[4];
    int *p = &a[1];
    int *q = &b[2];
    q = p + 2;
    return (int)(q - p);
}
```

