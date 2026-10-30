# Rule 9.1

**Category:** Mandatory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Initialization

## What the detector reports

Reports a read of an automatic object that the control-flow analysis finds uninitialized on every path (certain) or on some path (possible).

Detection contract: Report a read of an automatic object that the control-flow analysis finds uninitialized on every path (Definite) or on some path (Possible).

## Known limits

Only automatic scalar objects are tracked. Values written through a pointer, and elements of arrays, are not followed.

- **False negative:** struct member read before it is written (only scalar objects are tracked) (documented limit)

## Tested cases

20 of 21 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | initialized at the declaration | pass |
| Compliant (must stay silent) | assigned on every path before the read | pass |
| Compliant (must stay silent) | initialized through its address | pass |
| Violation (must be reported) | read of an object that was never written | pass |
| Violation (must be reported) | written on only one path | pass |
| Violation (must be reported) | read in an expression before the assignment | pass |
| Violation (must be reported) | increment of a never initialised object | pass |
| Compliant (must stay silent) | assigned in a for initialiser | pass |
| Compliant (must stay silent) | assigned by a do-while body before the read | pass |
| Compliant (must stay silent) | a static object is zero initialised | pass |
| Known false negative (not detected yet) | struct member read before it is written (only scalar objects are tracked) | documented limit |
| Compliant (must stay silent) | struct passed by address is initialised there | pass |
| Violation (must be reported) | switch with default that forgets to assign | pass |
| Compliant (must stay silent) | assigned in both arms of a conditional operator statement | pass |
| Compliant (must stay silent) | array with a complete initializer | pass |
| Compliant (must stay silent) | written in a loop and then read | pass |
| Compliant (must stay silent) | sizeof does not read the object | pass |
| Compliant (must stay silent) | assigned in every branch of an if/else chain | pass |
| Violation (must be reported) | the value is passed to a function before it is set | pass |
| Violation (must be reported) | a compound assignment reads first | pass |
| Violation (must be reported) | assigned only in some switch clauses without default | pass |

## Case sources

### Compliant (must stay silent): initialized at the declaration

`main.c`

```c
int f(void) { int x = 1; return x; }
```

### Compliant (must stay silent): assigned on every path before the read

`main.c`

```c
int f(int c)
{
    int x;
    if (c) { x = 1; } else { x = 2; }
    return x;
}
```

### Compliant (must stay silent): initialized through its address

`main.c`

```c
void init(int *p);
int f(void)
{
    int x;
    init(&x);
    return x;
}
```

### Violation (must be reported): read of an object that was never written

`main.c`

```c
int f(void)
{
    int x;
    return x; /*@V*/
}
```

### Violation (must be reported): written on only one path

`main.c`

```c
int f(int c)
{
    int x;
    if (c) { x = 1; }
    return x; /*@V*/
}
```

### Violation (must be reported): read in an expression before the assignment

`main.c`

```c
int f(void)
{
    int a;
    int b = a + 1; /*@V*/
    a = 2;
    return b + a;
}
```

### Violation (must be reported): increment of a never initialised object

`main.c`

```c
int f(void)
{
    int x;
    x++; /*@V*/
    return x;
}
```

### Compliant (must stay silent): assigned in a for initialiser

`main.c`

```c
int f(int n)
{
    int i;
    int s = 0;
    for (i = 0; i < n; i++) { s += i; }
    return s;
}
```

### Compliant (must stay silent): assigned by a do-while body before the read

`main.c`

```c
int f(void)
{
    int x;
    do { x = 1; } while (0);
    return x;
}
```

### Compliant (must stay silent): a static object is zero initialised

`main.c`

```c
int f(void)
{
    static int x;
    return x;
}
```

### Known false negative (not detected yet): struct member read before it is written (only scalar objects are tracked)

`main.c`

```c
struct s { int a; int b; };
int f(void)
{
    struct s v;
    v.a = 1;
    return v.b; /*@V*/
}
```

### Compliant (must stay silent): struct passed by address is initialised there

`main.c`

```c
struct s { int a; };
void init(struct s *p);
int f(void)
{
    struct s v;
    init(&v);
    return v.a;
}
```

### Violation (must be reported): switch with default that forgets to assign

`main.c`

```c
int f(int c)
{
    int x;
    switch (c)
    {
        case 1: x = 1; break;
        default: break;
    }
    return x; /*@V*/
}
```

### Compliant (must stay silent): assigned in both arms of a conditional operator statement

`main.c`

```c
int f(int c)
{
    int x;
    if (c) { x = 1; } else { x = 0; }
    return x + 1;
}
```

### Compliant (must stay silent): array with a complete initializer

`main.c`

```c
int f(void) { int a[3] = { 0 }; return a[1]; }
```

### Compliant (must stay silent): written in a loop and then read

`main.c`

```c
int f(int n)
{
    int s;
    int i;
    s = 0;
    for (i = 0; i < n; i++) { s += i; }
    return s;
}
```

### Compliant (must stay silent): sizeof does not read the object

`main.c`

```c
int f(void) { int x; return (int)sizeof(x); }
```

### Compliant (must stay silent): assigned in every branch of an if/else chain

`main.c`

```c
int f(int c)
{
    int x;
    if (c == 1) { x = 1; } else if (c == 2) { x = 2; } else { x = 3; }
    return x;
}
```

### Violation (must be reported): the value is passed to a function before it is set

`main.c`

```c
void use(int v);
void f(void)
{
    int x;
    use(x); /*@V*/
}
```

### Violation (must be reported): a compound assignment reads first

`main.c`

```c
int f(void)
{
    int x;
    x += 1; /*@V*/
    return x;
}
```

### Violation (must be reported): assigned only in some switch clauses without default

`main.c`

```c
int f(int c)
{
    int x;
    switch (c)
    {
        case 1: x = 1; break;
        case 2: x = 2; break;
    }
    return x; /*@V*/
}
```

