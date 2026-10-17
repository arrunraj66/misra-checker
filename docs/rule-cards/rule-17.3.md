# Rule 17.3

**Category:** Mandatory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Functions

## What the detector reports

Reports a call to an implicitly declared function (one with no prior declaration).

Detection contract: Report a call to an implicitly declared (non-builtin) function.

## Known limits

Compiler built-in functions are not reported.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | declared before the call | pass |
| Compliant (must stay silent) | a standard function with its header | pass |
| Violation (must be reported) | call of an undeclared function | pass |
| Violation (must be reported) | a second undeclared call | pass |
| Compliant (must stay silent) | a call through a function pointer and through a declared function | pass |
| Compliant (must stay silent) | compiler built-in functions are not reported | pass |
| Violation (must be reported) | a call in a condition | pass |

## Case sources

### Compliant (must stay silent): declared before the call

`main.c`

```c
int helper(int x);
int use(int x) { return helper(x); }
```

### Compliant (must stay silent): a standard function with its header

`main.c`

```c
#include <string.h>
unsigned long n(const char *s) { return strlen(s); }
```

### Violation (must be reported): call of an undeclared function

`main.c`

```c
int use(int x)
{
    return unknown_function(x); /*@V*/
}
```

### Violation (must be reported): a second undeclared call

`main.c`

```c
void g(void)
{
    another_unknown(); /*@V*/
}
```

### Compliant (must stay silent): a call through a function pointer and through a declared function

`main.c`

```c
int apply(int (*fn)(int), int v) { return fn(v); }
```

### Compliant (must stay silent): compiler built-in functions are not reported

`main.c`

```c
int f(int x) { return __builtin_expect(x, 1); }
```

### Violation (must be reported): a call in a condition

`main.c`

```c
int f(int x)
{
    if (mystery(x)) { return 1; } /*@V*/
    return 0;
}
```

