# Rule 18.8

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointers and arrays

## What the detector reports

Reports a declaration of variable-length array type.

Detection contract: Report a declaration of variable-length array type.

## Known limits

Only declarations whose size is not a constant are examined.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | constant sized arrays | pass |
| Violation (must be reported) | size from a parameter | pass |
| Violation (must be reported) | a variable-length array parameter type | pass |

## Case sources

### Compliant (must stay silent): constant sized arrays

`main.c`

```c
#define N 8
void f(void) { int a[N]; int b[4]; (void)a; (void)b; }
```

### Violation (must be reported): size from a parameter

`main.c`

```c
void f(int n)
{
    int a[n]; /*@V*/
    (void)a;
}
```

### Violation (must be reported): a variable-length array parameter type

`main.c`

```c
void g(int n, int m[n]); /*@V*/
```

