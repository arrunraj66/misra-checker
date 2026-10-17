# Rule 2.2

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Unused and unreachable code

## What the detector reports

Reports an expression statement without any effect (a cast to void is accepted) and an assignment to a local object whose value is never read afterwards.

Detection contract: Report an expression statement with no side effects (a cast to void is accepted) and an assignment to a local object whose value is never read (liveness analysis).

## Known limits

Dead stores are found for local objects with liveness analysis. Stores to globals, through pointers or to volatile objects are not examined.

- **False negative:** a store through a pointer is not tracked (documented limit)

## Tested cases

14 of 15 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a void cast marks the discard as intentional | pass |
| Compliant (must stay silent) | the stored value is used | pass |
| Compliant (must stay silent) | a call is an effect | pass |
| Violation (must be reported) | expression statement with no effect | pass |
| Violation (must be reported) | expression statement that is just a name | pass |
| Violation (must be reported) | the first assignment is overwritten before it is read | pass |
| Violation (must be reported) | a value computed and never used | pass |
| Compliant (must stay silent) | initializer that is read later | pass |
| Compliant (must stay silent) | initializer overwritten only after it was read | pass |
| Known false negative (not detected yet) | a store through a pointer is not tracked | documented limit |
| Compliant (must stay silent) | a volatile read is an effect | pass |
| Compliant (must stay silent) | increments and calls as statements | pass |
| Compliant (must stay silent) | a cast to void of a call result | pass |
| Violation (must be reported) | a comparison used as a statement | pass |
| Violation (must be reported) | a member access used as a statement | pass |

## Case sources

### Compliant (must stay silent): a void cast marks the discard as intentional

`main.c`

```c
void f(int x) { (void)x; }
```

### Compliant (must stay silent): the stored value is used

`main.c`

```c
int f(void)
{
    int a = 1;
    a = a + 2;
    return a;
}
```

### Compliant (must stay silent): a call is an effect

`main.c`

```c
void g(void);
void h(void) { g(); }
```

### Violation (must be reported): expression statement with no effect

`main.c`

```c
void f(int x)
{
    x + 1; /*@V*/
}
```

### Violation (must be reported): expression statement that is just a name

`main.c`

```c
void g(int x)
{
    x; /*@V*/
}
```

### Violation (must be reported): the first assignment is overwritten before it is read

`main.c`

```c
int f(void)
{
    int a;
    a = 1; /*@V*/
    a = 2;
    return a;
}
```

### Violation (must be reported): a value computed and never used

`main.c`

```c
int g(int x)
{
    int unused = x * 2; /*@V*/
    return x;
}
```

### Compliant (must stay silent): initializer that is read later

`main.c`

```c
int f(int x)
{
    int doubled = x * 2;
    return doubled;
}
```

### Compliant (must stay silent): initializer overwritten only after it was read

`main.c`

```c
int g(int x)
{
    int a = x;
    int b = a + 1;
    a = b;
    return a;
}
```

### Known false negative (not detected yet): a store through a pointer is not tracked

`main.c`

```c
void h(int *p)
{
    *p = 1; /*@V*/
    *p = 2;
}
```

### Compliant (must stay silent): a volatile read is an effect

`main.c`

```c
volatile int status;
void f(void) { (void)status; }
```

### Compliant (must stay silent): increments and calls as statements

`main.c`

```c
void g(void);
void f(int *p) { (*p)++; g(); }
```

### Compliant (must stay silent): a cast to void of a call result

`main.c`

```c
int g(void);
void f(void) { (void)g(); }
```

### Violation (must be reported): a comparison used as a statement

`main.c`

```c
void f(int a, int b)
{
    a == b; /*@V*/
}
```

### Violation (must be reported): a member access used as a statement

`main.c`

```c
struct s { int v; };
void f(struct s *p)
{
    p->v; /*@V*/
}
```

