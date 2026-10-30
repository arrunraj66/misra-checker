# Rule 13.5

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Side effects and evaluation order

## What the detector reports

Reports a right operand of && or || that has side effects, including calls and volatile accesses.

Detection contract: Report a right operand of && or || that has side effects (including calls and volatile access).

## Known limits

A call is assumed to have side effects. Reading a volatile object counts as a persistent side effect (the FreeRTOS authors rewrite such code, so the strict reading is followed).

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | pure right operands | pass |
| Violation (must be reported) | a call on the right | pass |
| Violation (must be reported) | an increment on the right | pass |
| Violation (must be reported) | an assignment on the right | pass |
| Violation (must be reported) | a volatile read on the right | pass |
| Compliant (must stay silent) | a pointer check protects the member read | pass |
| Violation (must be reported) | a decrement on the right of // | pass |
| Violation (must be reported) | a volatile structure member on the right | pass |
| Compliant (must stay silent) | both operands pure comparisons | pass |

## Case sources

### Compliant (must stay silent): pure right operands

`main.c`

```c
int f(int a, int b) { return a > 0 && b > 0; }
int g(int a, int b, int c) { return a || (b + c > 1); }
```

### Violation (must be reported): a call on the right

`main.c`

```c
int check(void);
int f(int a) { return a && check(); } /*@V*/
```

### Violation (must be reported): an increment on the right

`main.c`

```c
int g(int a, int b) { return a || b++; } /*@V*/
```

### Violation (must be reported): an assignment on the right

`main.c`

```c
int h(int a, int b) { return a && (b = 1); } /*@V*/
```

### Violation (must be reported): a volatile read on the right

`main.c`

```c
volatile int status;
int k(int a) { return a && status; } /*@V*/
```

### Compliant (must stay silent): a pointer check protects the member read

`main.c`

```c
struct s { int v; };
int f(struct s *p) { return (p != 0) && (p->v > 0); }
```

### Violation (must be reported): a decrement on the right of ||

`main.c`

```c
int f(int a, int b) { return (a > 0) || (b-- > 0); } /*@V*/
```

### Violation (must be reported): a volatile structure member on the right

`main.c`

```c
struct r { volatile int v; };
int f(struct r *p, int a) { return (a > 0) && (p->v > 0); } /*@V*/
```

### Compliant (must stay silent): both operands pure comparisons

`main.c`

```c
int f(int a, int b) { return (a > 0) && (b > 0) && (a != b); }
```

