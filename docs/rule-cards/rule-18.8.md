# Rule 18.8

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointers and arrays

## What the detector reports

Reports a declaration of variable-length array type.

Detection contract: Report a declaration of variable-length array type.

## Known limits

Only declarations whose size is not a constant are examined.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | constant sized arrays | pass |
| Violation (must be reported) | size from a parameter | pass |
| Violation (must be reported) | a variable-length array parameter type | pass |
| Violation (must be reported) | a variable length array in a nested block | pass |
| Compliant (must stay silent) | an enumeration constant as the size | pass |
| Compliant (must stay silent) | sizeof based constant size | pass |
| Violation (must be reported) | a variable length array parameter size | pass |
| Compliant (must stay silent) | a constant length array | pass |
| Compliant (must stay silent) | a pointer to a runtime sized buffer | pass |

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

### Violation (must be reported): a variable length array in a nested block

`main.c`

```c
void f(int n)
{
    if (n > 0)
    {
        int a[n]; /*@V*/
        a[0] = 1;
    }
}
```

### Compliant (must stay silent): an enumeration constant as the size

`main.c`

```c
enum { SIZE = 4 };
int a[SIZE];
```

### Compliant (must stay silent): sizeof based constant size

`main.c`

```c
int a[sizeof(int) * 2];
```

### Violation (must be reported): a variable length array parameter size

`main.c`

```c
void f(int n)
{
    int a[n]; /*@V*/
    a[0] = 0;
}
```

### Compliant (must stay silent): a constant length array

`main.c`

```c
void f(void)
{
    int a[8];
    a[0] = 0;
}
```

### Compliant (must stay silent): a pointer to a runtime sized buffer

`main.c`

```c
void f(int *buffer, int n)
{
    buffer[n - 1] = 0;
}
```

