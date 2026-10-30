# Rule 17.2

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Functions

## What the detector reports

Reports a function that can reach itself through direct or mutual calls visible in the translation unit.

Detection contract: Report a function that can reach itself through direct or mutual calls visible in the translation unit.

## Known limits

Calls through function pointers and calls into other translation units are not followed.

- **False negative:** recursion through a function pointer is not followed (documented limit)

## Tested cases

8 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no recursion | pass |
| Compliant (must stay silent) | a call to a different function with the same name pattern | pass |
| Violation (must be reported) | direct recursion | pass |
| Violation (must be reported) | mutual recursion | pass |
| Known false negative (not detected yet) | recursion through a function pointer is not followed | documented limit |
| Violation (must be reported) | a cycle through three functions | pass |
| Compliant (must stay silent) | a function calling a declared function that is not defined here | pass |
| Compliant (must stay silent) | same function name used as a variable elsewhere | pass |
| Violation (must be reported) | recursion in a static function | pass |

## Case sources

### Compliant (must stay silent): no recursion

`main.c`

```c
int helper(int x) { return x + 1; }
int top(int x) { return helper(x); }
```

### Compliant (must stay silent): a call to a different function with the same name pattern

`main.c`

```c
int fact_iter(int n) { int r = 1; int i; for (i = 2; i <= n; i++) { r *= i; } return r; }
```

### Violation (must be reported): direct recursion

`main.c`

```c
int fact(int n) /*@V*/
{
    return (n <= 1) ? 1 : (n * fact(n - 1));
}
```

### Violation (must be reported): mutual recursion

`main.c`

```c
int is_odd(int n);
int is_even(int n) /*@V*/
{
    return (n == 0) ? 1 : is_odd(n - 1);
}
int is_odd(int n)
{
    return (n == 0) ? 0 : is_even(n - 1);
}
```

### Known false negative (not detected yet): recursion through a function pointer is not followed

`main.c`

```c
typedef int (*fn_t)(int);
int loop(int n)
{
    fn_t self = loop;
    return (n > 0) ? self(n - 1) : 0; /*@V*/
}
```

### Violation (must be reported): a cycle through three functions

`main.c`

```c
int b(int n);
int c(int n);
int a(int n) { return (n > 0) ? b(n - 1) : 0; } /*@V*/
int b(int n) { return c(n); }
int c(int n) { return a(n); }
```

### Compliant (must stay silent): a function calling a declared function that is not defined here

`main.c`

```c
int ext(int n);
int f(int n) { return ext(n); }
```

### Compliant (must stay silent): same function name used as a variable elsewhere

`main.c`

```c
int count;
int f(void) { return count; }
```

### Violation (must be reported): recursion in a static function

`main.c`

```c
static int fact(int n) { return (n <= 1) ? 1 : n * fact(n - 1); } /*@V*/
int g(void) { return fact(3); }
```

