# Rule 13.1

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Side effects and evaluation order

## What the detector reports

Reports an initializer-list element that has side effects.

Detection contract: Report an initializer-list element that has side effects.

## Known limits

Only side effects that the compiler front end can see in the element expression are found.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | constants and pure expressions | pass |
| Compliant (must stay silent) | a pure expression in a local initializer | pass |
| Violation (must be reported) | an increment in an initializer list | pass |
| Violation (must be reported) | an assignment in an initializer list | pass |
| Violation (must be reported) | a call with side effects in an initializer list | pass |

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

