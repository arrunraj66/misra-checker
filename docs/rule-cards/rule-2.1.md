# Rule 2.1

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Unused and unreachable code

## What the detector reports

Reports the first statement of each region that the control-flow graph shows can never be reached.

Detection contract: Report the first code statement of each region that the control-flow graph shows cannot be reached.

## Known limits

Reachability follows the constant conditions the compiler front end folds. Code made unreachable only by run-time values is not found, and a region is reported once, at its first statement.

## Tested cases

12 of 12 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | both branches are reachable | pass |
| Compliant (must stay silent) | code after an infinite loop with a break is reachable | pass |
| Compliant (must stay silent) | every switch clause is reachable | pass |
| Violation (must be reported) | statement after return | pass |
| Violation (must be reported) | statement after goto | pass |
| Violation (must be reported) | code after an infinite loop without break | pass |
| Violation (must be reported) | statements after a call to a no-return construct (infinite loop in a while) | pass |
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

