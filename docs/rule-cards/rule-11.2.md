# Rule 11.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports a conversion between a pointer to an incomplete type and any other pointer or integer type (a null pointer constant is exempt).

Detection contract: Report a conversion between a pointer to an incomplete type and any other pointer or integer type.

## Known limits

Only conversions of pointers to incomplete struct or union types are examined.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a null pointer constant and equal types | pass |
| Compliant (must stay silent) | conversion to void pointer of a complete type | pass |
| Violation (must be reported) | pointer to incomplete type cast to an integer | pass |
| Violation (must be reported) | pointer to incomplete type cast to another pointer type | pass |

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

