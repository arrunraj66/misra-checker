# Rule 2.2

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Unused and unreachable code

## What the detector reports

Reports an expression statement without any effect (a cast to void is accepted) and an assignment to a local object whose value is never read afterwards.

Detection contract: Report an expression statement with no side effects (a cast to void is accepted) and an assignment to a local object whose value is never read (liveness analysis).

## Known limits

Dead stores are found for local objects with liveness analysis. Stores to globals, through pointers or to volatile objects are not examined.

- **False negative:** a store through a pointer is not tracked (documented limit)

## Tested cases

26 of 27 pass.

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
| Violation (must be reported) | self assignment has no effect | pass |
| Compliant (must stay silent) | a value read in the next loop iteration | pass |
| Compliant (must stay silent) | assignment to a static local persists | pass |
| Compliant (must stay silent) | assignment to a file-scope object | pass |
| Compliant (must stay silent) | a local whose address is taken may be read through the pointer | pass |
| Violation (must be reported) | assignment before a return that ignores it | pass |
| Compliant (must stay silent) | assignment read on only one path | pass |
| Compliant (must stay silent) | out parameter pointer write | pass |
| Compliant (must stay silent) | both branches assign then read | pass |
| Violation (must be reported) | compound assignment whose result is never read | pass |
| Compliant (must stay silent) | assignment used as a condition | pass |
| Compliant (must stay silent) | a local array element store is not tracked | pass |
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

### Violation (must be reported): self assignment has no effect

`main.c`

```c
void f(int a)
{
    int x = a;
    x = x; /*@V*/
}
```

### Compliant (must stay silent): a value read in the next loop iteration

`main.c`

```c
int f(int n)
{
    int prev = 0;
    int i;
    for (i = 0; i < n; i++)
    {
        n += prev;
        prev = i;
    }
    return n;
}
```

### Compliant (must stay silent): assignment to a static local persists

`main.c`

```c
int f(void)
{
    static int count;
    count = count + 1;
    count = 0;
    return 1;
}
```

### Compliant (must stay silent): assignment to a file-scope object

`main.c`

```c
int g;
void f(void) { g = 1; g = 2; }
```

### Compliant (must stay silent): a local whose address is taken may be read through the pointer

`main.c`

```c
void use(int *p);
void f(void)
{
    int x;
    x = 1;
    use(&x);
}
```

### Violation (must be reported): assignment before a return that ignores it

`main.c`

```c
int f(void)
{
    int x;
    x = 5; /*@V*/
    return 0;
}
```

### Compliant (must stay silent): assignment read on only one path

`main.c`

```c
int f(int c)
{
    int x = 0;
    if (c) { x = 1; }
    return x;
}
```

### Compliant (must stay silent): out parameter pointer write

`main.c`

```c
void f(int *out) { *out = 1; }
```

### Compliant (must stay silent): both branches assign then read

`main.c`

```c
int f(int c)
{
    int x;
    if (c) { x = 1; } else { x = 2; }
    return x;
}
```

### Violation (must be reported): compound assignment whose result is never read

`main.c`

```c
void f(void)
{
    int x = 0;
    x += 5; /*@V*/
}
```

### Compliant (must stay silent): assignment used as a condition

`main.c`

```c
int f(int *p)
{
    int c;
    while ((c = *p++) != 0) { }
    return c;
}
```

### Compliant (must stay silent): a local array element store is not tracked

`main.c`

```c
void f(void)
{
    int a[2];
    a[0] = 1;
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

