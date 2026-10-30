# Rule 8.13

**Category:** Advisory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Declarations and definitions

## What the detector reports

Subset: reports a pointer parameter to non-const whose every use in the function body is read-only.

Detection contract: Subset: report a pointer parameter to non-const whose every use in the function is provably read-only.

## Known limits

Only direct uses in the function are examined; a pointer passed to another function is assumed to be modified.

- **False negative:** pointer to pointer that is only read (only the first level of pointers is examined) (documented limit)

## Tested cases

23 of 24 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a pointer to const | pass |
| Compliant (must stay silent) | the pointee is modified | pass |
| Compliant (must stay silent) | the pointer is passed on (it may be modified there) | pass |
| Violation (must be reported) | read-only use of a non-const pointer parameter | pass |
| Violation (must be reported) | indexed read-only use | pass |
| Violation (must be reported) | a pointer only compared with null | pass |
| Known false negative (not detected yet) | pointer to pointer that is only read (only the first level of pointers is examined) | documented limit |
| Compliant (must stay silent) | a pointer stored into a pointer to non-const | pass |
| Compliant (must stay silent) | a pointer passed to a function that may write | pass |
| Violation (must be reported) | volatile register read only | pass |
| Compliant (must stay silent) | function pointer parameters are not data pointers | pass |
| Violation (must be reported) | pointer to a const-qualified struct member read | pass |
| Compliant (must stay silent) | written through a member | pass |
| Compliant (must stay silent) | member of the pointee modified through increment | pass |
| Violation (must be reported) | a pointer that is advanced and read, never written through | pass |
| Violation (must be reported) | a structure pointer that is only read | pass |
| Compliant (must stay silent) | a structure pointer that is written through | pass |
| Compliant (must stay silent) | both pointers are written through | pass |
| Compliant (must stay silent) | the pointer is returned as a pointer to non-const | pass |
| Compliant (must stay silent) | written through an index | pass |
| Compliant (must stay silent) | read-modify-write through the pointer | pass |
| Violation (must be reported) | a pointer passed to a function that only needs const | pass |
| Compliant (must stay silent) | a pointer advanced and written through | pass |
| Compliant (must stay silent) | a pointer passed on to a function that may modify it | pass |

## Case sources

### Compliant (must stay silent): a pointer to const

`main.c`

```c
int sum(const int *p, int n) { int s = 0; int i; for (i = 0; i < n; i++) { s += p[i]; } return s; }
```

### Compliant (must stay silent): the pointee is modified

`main.c`

```c
void clear(int *p) { *p = 0; }
```

### Compliant (must stay silent): the pointer is passed on (it may be modified there)

`main.c`

```c
void sink(int *p);
void forward(int *p) { sink(p); }
```

### Violation (must be reported): read-only use of a non-const pointer parameter

`main.c`

```c
int first(int *p) { return *p; } /*@V*/
```

### Violation (must be reported): indexed read-only use

`main.c`

```c
int at(int *a, int i) { return a[i]; } /*@V*/
```

### Violation (must be reported): a pointer only compared with null

`main.c`

```c
int f(int *p) { return p != 0; } /*@V*/
```

### Known false negative (not detected yet): pointer to pointer that is only read (only the first level of pointers is examined)

`main.c`

```c
int f(char **argv) { return argv[0][0]; } /*@V*/
```

### Compliant (must stay silent): a pointer stored into a pointer to non-const

`main.c`

```c
int *g;
void f(int *p) { g = p; }
```

### Compliant (must stay silent): a pointer passed to a function that may write

`main.c`

```c
void fill(int *out);
void f(int *p) { fill(p); }
```

### Violation (must be reported): volatile register read only

`main.c`

```c
int f(volatile int *reg) { return *reg; } /*@V*/
```

### Compliant (must stay silent): function pointer parameters are not data pointers

`main.c`

```c
int f(int (*cb)(int)) { return cb(1); }
```

### Violation (must be reported): pointer to a const-qualified struct member read

`main.c`

```c
struct s { int a; };
int f(struct s *p) { return p->a + 1; } /*@V*/
```

### Compliant (must stay silent): written through a member

`main.c`

```c
struct s { int a; };
void f(struct s *p) { p->a = 1; }
```

### Compliant (must stay silent): member of the pointee modified through increment

`main.c`

```c
struct s { int a; };
void f(struct s *p) { p->a++; }
```

### Violation (must be reported): a pointer that is advanced and read, never written through

`main.c`

```c
int sum(int *p, int n) /*@V*/
{
    int s = 0;
    while (n-- > 0) { s += *p++; }
    return s;
}
```

### Violation (must be reported): a structure pointer that is only read

`main.c`

```c
struct pair { int x; int y; };
int first(struct pair *p) { return p->x; } /*@V*/
```

### Compliant (must stay silent): a structure pointer that is written through

`main.c`

```c
struct pair { int x; int y; };
void set(struct pair *p) { p->x = 1; }
```

### Compliant (must stay silent): both pointers are written through

`main.c`

```c
void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }
```

### Compliant (must stay silent): the pointer is returned as a pointer to non-const

`main.c`

```c
int *same(int *p) { return p; }
```

### Compliant (must stay silent): written through an index

`main.c`

```c
void fill(int *p) { p[0] = 1; }
```

### Compliant (must stay silent): read-modify-write through the pointer

`main.c`

```c
void inc(int *p) { (*p)++; }
```

### Violation (must be reported): a pointer passed to a function that only needs const

`main.c`

```c
int peek(const int *p);
int use(int *p) { return peek(p); } /*@V*/
```

### Compliant (must stay silent): a pointer advanced and written through

`main.c`

```c
void zero(int *p, int n) { while (n-- > 0) { *p++ = 0; } }
```

### Compliant (must stay silent): a pointer passed on to a function that may modify it

`main.c`

```c
void sink(int *p);
int forward(int *p) { sink(p); return 0; }
```

