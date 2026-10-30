# Rule 11.5

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports a conversion from pointer to void to a pointer to an object type, implicit or explicit.

Detection contract: Report a conversion from pointer-to-void to pointer-to-object, implicit or explicit.

## Known limits

A conversion in the other direction (object pointer to void pointer) is compliant.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | object pointer to void pointer | pass |
| Compliant (must stay silent) | void pointer stays void pointer | pass |
| Violation (must be reported) | implicit conversion | pass |
| Violation (must be reported) | explicit cast of a void pointer | pass |
| Compliant (must stay silent) | object pointer passed to a void pointer parameter | pass |
| Violation (must be reported) | void pointer returned as an object pointer | pass |
| Compliant (must stay silent) | comparison of a void pointer with an object pointer | pass |
| Violation (must be reported) | the result of a function returning void pointer assigned to an int pointer | pass |
| Compliant (must stay silent) | stored in another void pointer | pass |

## Case sources

### Compliant (must stay silent): object pointer to void pointer

`main.c`

```c
int x;
void *v = &x;
```

### Compliant (must stay silent): void pointer stays void pointer

`main.c`

```c
void *copy(void *p) { return p; }
```

### Violation (must be reported): implicit conversion

`main.c`

```c
void *src(void);
int *p;
void f(void) { p = src(); } /*@V*/
```

### Violation (must be reported): explicit cast of a void pointer

`main.c`

```c
void *src(void);
int *g(void) { return (int *)src(); } /*@V*/
```

### Compliant (must stay silent): object pointer passed to a void pointer parameter

`main.c`

```c
void use(void *p);
void f(int *q) { use(q); }
```

### Violation (must be reported): void pointer returned as an object pointer

`main.c`

```c
void *src(void);
int *f(void) { return src(); } /*@V*/
```

### Compliant (must stay silent): comparison of a void pointer with an object pointer

`main.c`

```c
int f(void *v, int *p) { return v == p; }
```

### Violation (must be reported): the result of a function returning void pointer assigned to an int pointer

`main.c`

```c
void *get(void);
void f(void)
{
    int *p = get(); /*@V*/
    (void)p;
}
```

### Compliant (must stay silent): stored in another void pointer

`main.c`

```c
void *get(void);
void f(void)
{
    void *p = get();
    (void)p;
}
```

