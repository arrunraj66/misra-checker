# Rule 2.1

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Unused and unreachable code

## What the detector reports

Reports the first statement of each region that the control-flow graph shows can never be reached.

Detection contract: Report the first code statement of each region that the control-flow graph shows cannot be reached.

## Known limits

Reachability follows the constant conditions the compiler front end folds. Code made unreachable only by run-time values is not found, and a region is reported once, at its first statement.

## Tested cases

24 of 24 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | both branches are reachable | pass |
| Compliant (must stay silent) | code after an infinite loop with a break is reachable | pass |
| Compliant (must stay silent) | every switch clause is reachable | pass |
| Violation (must be reported) | statement after return | pass |
| Violation (must be reported) | statement after goto | pass |
| Violation (must be reported) | code after an infinite loop without break | pass |
| Violation (must be reported) | statements after a call to a no-return construct (infinite loop in a while) | pass |
| Violation (must be reported) | code after break inside a loop body | pass |
| Violation (must be reported) | code after continue | pass |
| Violation (must be reported) | statement after a switch whose every branch returns | pass |
| Compliant (must stay silent) | statement after a switch with a fall-out branch | pass |
| Compliant (must stay silent) | a label reached only by a goto is reachable | pass |
| Violation (must be reported) | a label after return that nothing jumps to | pass |
| Violation (must be reported) | body of if(0) | pass |
| Compliant (must stay silent) | a defensive default after a switch over every enumerator | pass |
| Compliant (must stay silent) | code after a call to a function that may return | pass |
| Compliant (must stay silent) | for(;;) with a return inside and nothing after | pass |
| Violation (must be reported) | statement after for(;;) without exit | pass |
| Compliant (must stay silent) | statement after a while loop with a non-constant condition | pass |
| Compliant (must stay silent) | code after a branch that returns on both arms is not reached (no code follows) | pass |
| Compliant (must stay silent) | a loop that ends through a condition | pass |
| Compliant (must stay silent) | do-while-zero in a macro is reachable | pass |
| Violation (must be reported) | code after an infinite while in a void function | pass |
| Violation (must be reported) | the else branch of a constant true condition | pass |

## Case sources

### Compliant (must stay silent): both branches are reachable

`main.c`

```c
int f(int v)
{
    if (v > 0) { return 1; }
    return 0;
}
```

### Compliant (must stay silent): code after an infinite loop with a break is reachable

`main.c`

```c
int f(int v)
{
    for (;;)
    {
        if (v > 3) { break; }
        v++;
    }
    return v;
}
```

### Compliant (must stay silent): every switch clause is reachable

`main.c`

```c
int f(int v)
{
    switch (v)
    {
        case 1: return 10;
        default: return 0;
    }
}
```

### Violation (must be reported): statement after return

`main.c`

```c
int f(void)
{
    return 1;
    return 2; /*@V*/
}
```

### Violation (must be reported): statement after goto

`main.c`

```c
void g(int *p)
{
    goto end;
    *p = 1; /*@V*/
end:
    return;
}
```

### Violation (must be reported): code after an infinite loop without break

`main.c`

```c
int h(void)
{
    for (;;) { }
    return 1; /*@V*/
}
```

### Violation (must be reported): statements after a call to a no-return construct (infinite loop in a while)

`main.c`

```c
void k(void)
{
    while (1) { }
    k(); /*@V*/
}
```

### Violation (must be reported): code after break inside a loop body

`main.c`

```c
int f(int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        break;
        n--; /*@V*/
    }
    return i;
}
```

### Violation (must be reported): code after continue

`main.c`

```c
int f(int n)
{
    int s = 0;
    while (n > 0)
    {
        n--;
        continue;
        s++; /*@V*/
    }
    return s;
}
```

### Violation (must be reported): statement after a switch whose every branch returns

`main.c`

```c
int f(int v)
{
    switch (v)
    {
        case 1: return 1;
        default: return 0;
    }
    return 2; /*@V*/
}
```

### Compliant (must stay silent): statement after a switch with a fall-out branch

`main.c`

```c
int f(int v)
{
    int r = 0;
    switch (v)
    {
        case 1: r = 1; break;
        default: break;
    }
    return r;
}
```

### Compliant (must stay silent): a label reached only by a goto is reachable

`main.c`

```c
int f(int v)
{
    if (v) { goto done; }
    v = 3;
done:
    return v;
}
```

### Violation (must be reported): a label after return that nothing jumps to

`main.c`

```c
int f(int v)
{
    return v;
fin:
    v = 0; /*@V*/
    return v;
}
```

### Violation (must be reported): body of if(0)

`main.c`

```c
int f(int v)
{
    if (0)
    {
        v = 1; /*@V*/
    }
    return v;
}
```

### Compliant (must stay silent): a defensive default after a switch over every enumerator

`main.c`

```c
enum e { A, B };
int f(enum e v)
{
    switch (v)
    {
        case A: return 1;
        case B: return 2;
        default: return 0;
    }
}
```

### Compliant (must stay silent): code after a call to a function that may return

`main.c`

```c
void g(void);
void f(void) { g(); g(); }
```

### Compliant (must stay silent): for(;;) with a return inside and nothing after

`main.c`

```c
int f(int *p)
{
    for (;;)
    {
        if (*p) { return 1; }
        p++;
    }
}
```

### Violation (must be reported): statement after for(;;) without exit

`main.c`

```c
void f(int *p)
{
    for (;;) { (*p)++; }
    *p = 0; /*@V*/
}
```

### Compliant (must stay silent): statement after a while loop with a non-constant condition

`main.c`

```c
void f(int *p)
{
    while (*p) { p++; }
    *p = 1;
}
```

### Compliant (must stay silent): code after a branch that returns on both arms is not reached (no code follows)

`main.c`

```c
int f(int c) { if (c) { return 1; } else { return 2; } }
```

### Compliant (must stay silent): a loop that ends through a condition

`main.c`

```c
int f(int n) { int i = 0; while (i < n) { i++; } return i; }
```

### Compliant (must stay silent): do-while-zero in a macro is reachable

`main.c`

```c
#define STEP(p) do { (*(p))++; } while (0)
void f(int *p) { STEP(p); }
```

### Violation (must be reported): code after an infinite while in a void function

`main.c`

```c
void f(int *p)
{
    while (1) { (*p)++; }
    *p = 0; /*@V*/
}
```

### Violation (must be reported): the else branch of a constant true condition

`main.c`

```c
int f(void)
{
    if (1) { return 1; } else { return 2; } /*@V*/
}
```

