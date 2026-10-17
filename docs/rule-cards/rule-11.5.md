# Rule 11.5

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports a conversion from pointer to void to a pointer to an object type, implicit or explicit.

Detection contract: Report a conversion from pointer-to-void to pointer-to-object, implicit or explicit.

## Known limits

A conversion in the other direction (object pointer to void pointer) is compliant.

## Tested cases

6 of 6 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | object pointer to void pointer | pass |
| Compliant (must stay silent) | void pointer stays void pointer | pass |
| Violation (must be reported) | implicit conversion | pass |
| Violation (must be reported) | explicit cast of a void pointer | pass |
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

