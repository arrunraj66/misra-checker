# Rule 11.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports an explicit cast between a pointer to void and an integer type.

Detection contract: Report an explicit cast between pointer-to-void and an integer type.

## Known limits

A null pointer constant is exempt.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | null pointer | pass |
| Compliant (must stay silent) | pointers only | pass |
| Violation (must be reported) | integer cast to void pointer | pass |
| Violation (must be reported) | void pointer cast to integer | pass |
| Violation (must be reported) | a nonzero integer cast to void pointer | pass |
| Compliant (must stay silent) | the null pointer cast to void pointer | pass |
| Violation (must be reported) | void pointer cast to an integer | pass |
| Violation (must be reported) | an integer cast to a void pointer | pass |
| Compliant (must stay silent) | a void pointer cast to an object pointer | pass |

## Case sources

### Compliant (must stay silent): null pointer

`main.c`

```c
void *p = (void *)0;
```

### Compliant (must stay silent): pointers only

`main.c`

```c
int x;
void *q = &x;
```

### Violation (must be reported): integer cast to void pointer

`main.c`

```c
void *p = (void *)5; /*@V*/
```

### Violation (must be reported): void pointer cast to integer

`main.c`

```c
unsigned long f(void *p) { return (unsigned long)p; } /*@V*/
```

### Violation (must be reported): a nonzero integer cast to void pointer

`main.c`

```c
void *f(void) { return (void *)8; } /*@V*/
```

### Compliant (must stay silent): the null pointer cast to void pointer

`main.c`

```c
void *f(void) { return (void *)0; }
```

### Violation (must be reported): void pointer cast to an integer

`main.c`

```c
void f(void *p)
{
    unsigned long v = (unsigned long)p; /*@V*/
    (void)v;
}
```

### Violation (must be reported): an integer cast to a void pointer

`main.c`

```c
void *f(unsigned long v)
{
    return (void *)v; /*@V*/
}
```

### Compliant (must stay silent): a void pointer cast to an object pointer

`main.c`

```c
int *f(void *p) { return (int *)p; }
```

