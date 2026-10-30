# Rule 11.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports a conversion between a pointer to an incomplete type and any other pointer or integer type (a null pointer constant is exempt).

Detection contract: Report a conversion between a pointer to an incomplete type and any other pointer or integer type.

## Known limits

Only conversions of pointers to incomplete struct or union types are examined.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a null pointer constant and equal types | pass |
| Compliant (must stay silent) | conversion to void pointer of a complete type | pass |
| Violation (must be reported) | pointer to incomplete type cast to an integer | pass |
| Violation (must be reported) | pointer to incomplete type cast to another pointer type | pass |
| Compliant (must stay silent) | a pointer to an incomplete type stored and returned unchanged | pass |
| Violation (must be reported) | incomplete pointer cast to char pointer | pass |
| Compliant (must stay silent) | a pointer to a complete struct converted through void | pass |
| Compliant (must stay silent) | a pointer to an incomplete type kept as the same type | pass |
| Violation (must be reported) | an incomplete struct pointer cast to an integer pointer | pass |

## Case sources

### Compliant (must stay silent): a null pointer constant and equal types

`main.c`

```c
struct opaque;
struct opaque *handle;
int is_set(void) { return handle != 0; }
struct opaque *copy(struct opaque *p) { return p; }
```

### Compliant (must stay silent): conversion to void pointer of a complete type

`main.c`

```c
int x;
void *v = &x;
```

### Violation (must be reported): pointer to incomplete type cast to an integer

`main.c`

```c
struct opaque;
unsigned long f(struct opaque *p) { return (unsigned long)p; } /*@V*/
```

### Violation (must be reported): pointer to incomplete type cast to another pointer type

`main.c`

```c
struct opaque;
char *g(struct opaque *p) { return (char *)p; } /*@V*/
```

### Compliant (must stay silent): a pointer to an incomplete type stored and returned unchanged

`main.c`

```c
struct opaque;
struct opaque *keep(struct opaque *p) { return p; }
```

### Violation (must be reported): incomplete pointer cast to char pointer

`main.c`

```c
struct opaque;
char *f(struct opaque *p) { return (char *)p; } /*@V*/
```

### Compliant (must stay silent): a pointer to a complete struct converted through void

`main.c`

```c
struct s { int a; };
void f(void *p)
{
    struct s *q = (struct s *)p;
    (void)q;
}
```

### Compliant (must stay silent): a pointer to an incomplete type kept as the same type

`main.c`

```c
struct opaque;
struct opaque *f(struct opaque *p) { return p; }
```

### Violation (must be reported): an incomplete struct pointer cast to an integer pointer

`main.c`

```c
struct opaque;
int *f(struct opaque *p)
{
    return (int *)p; /*@V*/
}
```

