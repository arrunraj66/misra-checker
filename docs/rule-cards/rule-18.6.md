# Rule 18.6

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Pointers and arrays

## What the detector reports

Subset: reports returning, or storing in a longer-lived object, the address of an automatic object.

Detection contract: Subset: report returning, or storing in longer-lived storage, the address of an automatic object.

## Known limits

Only direct address-of expressions of local objects are tracked.

- **False negative:** returning the address of a local through a pointer variable (only direct &local is tracked) (documented limit)

## Tested cases

11 of 12 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | the address of a static object or of a parameter pointer | pass |
| Compliant (must stay silent) | a local pointer to a local object | pass |
| Violation (must be reported) | returning the address of a local | pass |
| Violation (must be reported) | storing the address of a local in a global | pass |
| Violation (must be reported) | returning the address of a local array | pass |
| Violation (must be reported) | storing the address of a local through an output parameter | pass |
| Violation (must be reported) | returning the address of a parameter | pass |
| Compliant (must stay silent) | returning the address of a static local | pass |
| Compliant (must stay silent) | a local pointer receives a local address | pass |
| Compliant (must stay silent) | returning a pointer that was passed in | pass |
| Compliant (must stay silent) | returning the address of a file-scope object | pass |
| Known false negative (not detected yet) | returning the address of a local through a pointer variable (only direct &local is tracked) | documented limit |

## Case sources

### Compliant (must stay silent): the address of a static object or of a parameter pointer

`main.c`

```c
int *f(void) { static int s; return &s; }
int *g(int *p) { return p; }
```

### Compliant (must stay silent): a local pointer to a local object

`main.c`

```c
int h(void) { int x = 1; int *p = &x; return *p; }
```

### Violation (must be reported): returning the address of a local

`main.c`

```c
int *f(void)
{
    int x = 1;
    return &x; /*@V*/
}
```

### Violation (must be reported): storing the address of a local in a global

`main.c`

```c
int *keep;
void g(void)
{
    int y = 2;
    keep = &y; /*@V*/
}
```

### Violation (must be reported): returning the address of a local array

`main.c`

```c
char *name(void)
{
    char buf[8];
    return buf; /*@V*/
}
```

### Violation (must be reported): storing the address of a local through an output parameter

`main.c`

```c
void f(int **out)
{
    int x = 0;
    *out = &x; /*@V*/
}
```

### Violation (must be reported): returning the address of a parameter

`main.c`

```c
int *f(int a) { return &a; } /*@V*/
```

### Compliant (must stay silent): returning the address of a static local

`main.c`

```c
int *f(void)
{
    static int x;
    return &x;
}
```

### Compliant (must stay silent): a local pointer receives a local address

`main.c`

```c
int f(void)
{
    int x = 1;
    int *p = &x;
    return *p;
}
```

### Compliant (must stay silent): returning a pointer that was passed in

`main.c`

```c
char *f(char *buf) { buf[0] = 'a'; return buf; }
```

### Compliant (must stay silent): returning the address of a file-scope object

`main.c`

```c
int global;
int *f(void) { return &global; }
```

### Known false negative (not detected yet): returning the address of a local through a pointer variable (only direct &local is tracked)

`main.c`

```c
int *f(void)
{
    int x = 1;
    int *p = &x;
    return p; /*@V*/
}
```

