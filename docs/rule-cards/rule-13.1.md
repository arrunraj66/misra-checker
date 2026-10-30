# Rule 13.1

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Side effects and evaluation order

## What the detector reports

Reports an initializer-list element that has side effects.

Detection contract: Report an initializer-list element that has side effects.

## Known limits

Only side effects that the compiler front end can see in the element expression are found.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | constants and pure expressions | pass |
| Compliant (must stay silent) | a pure expression in a local initializer | pass |
| Violation (must be reported) | an increment in an initializer list | pass |
| Violation (must be reported) | an assignment in an initializer list | pass |
| Violation (must be reported) | a call with side effects in an initializer list | pass |
| Compliant (must stay silent) | constants, sizeof and arithmetic in an initializer list | pass |
| Violation (must be reported) | a volatile read in an initializer list | pass |
| Violation (must be reported) | a decrement in a structure initializer | pass |
| Compliant (must stay silent) | a parenthesised pure expression | pass |

## Case sources

### Compliant (must stay silent): constants and pure expressions

`main.c`

```c
int base = 2;
int a[3] = { 1, 2, 3 };
int b[2] = { 4, 5 };
```

### Compliant (must stay silent): a pure expression in a local initializer

`main.c`

```c
int f(int x) { int arr[2] = { x, x + 1 }; return arr[0] + arr[1]; }
```

### Violation (must be reported): an increment in an initializer list

`main.c`

```c
int f(int i)
{
    int arr[2] = { i++, 0 }; /*@V*/
    return arr[0];
}
```

### Violation (must be reported): an assignment in an initializer list

`main.c`

```c
int g(int i)
{
    int arr[2] = { i = 1, 0 }; /*@V*/
    return arr[0];
}
```

### Violation (must be reported): a call with side effects in an initializer list

`main.c`

```c
int next(void);
int h(void)
{
    int arr[2] = { next(), 0 }; /*@V*/
    return arr[0];
}
```

### Compliant (must stay silent): constants, sizeof and arithmetic in an initializer list

`main.c`

```c
int a = 1, b = 2;
int t[3] = { 1, sizeof(int), 2 + 3 };
```

### Violation (must be reported): a volatile read in an initializer list

`main.c`

```c
volatile int reg;
void f(void)
{
    int t[2] = { reg, 1 }; /*@V*/
    (void)t;
}
```

### Violation (must be reported): a decrement in a structure initializer

`main.c`

```c
struct s { int a; int b; };
void f(int n)
{
    struct s v = { n--, 1 }; /*@V*/
    (void)v;
}
```

### Compliant (must stay silent): a parenthesised pure expression

`main.c`

```c
void f(int a, int b)
{
    int t[2] = { (a + b) * 2, a - b };
    (void)t;
}
```

