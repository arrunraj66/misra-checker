# Rule 1.3

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Standard C conformance

## What the detector reports

Subset of undefined behaviour that is visible at compile time: constant division by zero, a constant array index outside the array, dereference of a null pointer constant, and out-of-range constant shifts.

Detection contract: Subset: report constant division by zero, constant array indexes outside the array, dereference of a null pointer constant and out-of-range constant shifts.

## Known limits

Only constants are examined. Values that depend on run-time data or on flow between statements are not tracked.

- **False negative:** a pointer variable that is known to be null (documented limit)
- **False negative:** division by a zero that is only known through a variable (documented limit)

## Tested cases

27 of 29 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | in-range constants and a guarded division | pass |
| Violation (must be reported) | division by a constant zero | pass |
| Violation (must be reported) | constant index past the end | pass |
| Violation (must be reported) | constant shift count that is too large | pass |
| Violation (must be reported) | dereference of a null pointer constant | pass |
| Compliant (must stay silent) | a memory-mapped register at a fixed non-zero address | pass |
| Compliant (must stay silent) | dereference of a pointer that was checked against null | pass |
| Known false negative (not detected yet) | a pointer variable that is known to be null | documented limit |
| Known false negative (not detected yet) | division by a zero that is only known through a variable | documented limit |
| Violation (must be reported) | modulo by a constant zero | pass |
| Violation (must be reported) | compound division by a constant zero | pass |
| Violation (must be reported) | negative constant index | pass |
| Violation (must be reported) | index equal to the array size | pass |
| Compliant (must stay silent) | last valid index and the address one past the end | pass |
| Violation (must be reported) | shift count equal to the width of int | pass |
| Compliant (must stay silent) | shift count one less than the width | pass |
| Violation (must be reported) | negative shift count | pass |
| Compliant (must stay silent) | offsetof idiom through a null pointer is not a dereference | pass |
| Violation (must be reported) | member access through a null pointer constant | pass |
| Compliant (must stay silent) | division where zero is excluded by the type of the divisor constant | pass |
| Compliant (must stay silent) | two-dimensional index in range | pass |
| Violation (must be reported) | second dimension index out of range | pass |
| Compliant (must stay silent) | index into a pointer parameter is unknown | pass |
| Compliant (must stay silent) | zero literal as the dividend | pass |
| Violation (must be reported) | null dereference through the NULL macro | pass |
| Compliant (must stay silent) | division by a macro that is non-zero | pass |
| Violation (must be reported) | division by a macro that is zero | pass |
| Violation (must be reported) | constant index into a string literal past its end | pass |
| Compliant (must stay silent) | index at the terminator of a string literal | pass |

## Case sources

### Compliant (must stay silent): in-range constants and a guarded division

`main.c`

```c
int arr[4];
int f(int d) { arr[3] = 1; return d != 0 ? 8 / d : 0; }
```

### Violation (must be reported): division by a constant zero

`main.c`

```c
int f(void) { return 10 / 0; } /*@V*/
```

### Violation (must be reported): constant index past the end

`main.c`

```c
int arr[4];
int f(void) { return arr[4]; } /*@V*/
```

### Violation (must be reported): constant shift count that is too large

`main.c`

```c
int f(int v) { return v << 40; } /*@V*/
```

### Violation (must be reported): dereference of a null pointer constant

`main.c`

```c
int f(void) { return *(int *)0; } /*@V*/
```

### Compliant (must stay silent): a memory-mapped register at a fixed non-zero address

`main.c`

```c
int read_register(void) { return *(volatile int *)0x4000; }
```

### Compliant (must stay silent): dereference of a pointer that was checked against null

`main.c`

```c
int f(int *p) { return (p != 0) ? *p : 0; }
```

### Known false negative (not detected yet): a pointer variable that is known to be null

`main.c`

```c
int f(void)
{
    int *p = 0;
    return *p; /*@V*/
}
```

### Known false negative (not detected yet): division by a zero that is only known through a variable

`main.c`

```c
int f(void)
{
    int zero = 0;
    return 10 / zero; /*@V*/
}
```

### Violation (must be reported): modulo by a constant zero

`main.c`

```c
int f(int a) { return a % 0; } /*@V*/
```

### Violation (must be reported): compound division by a constant zero

`main.c`

```c
void f(int *a) { *a /= 0; } /*@V*/
```

### Violation (must be reported): negative constant index

`main.c`

```c
int a[4];
int f(void) { return a[-1]; } /*@V*/
```

### Violation (must be reported): index equal to the array size

`main.c`

```c
int a[4];
int f(void) { return a[4]; } /*@V*/
```

### Compliant (must stay silent): last valid index and the address one past the end

`main.c`

```c
int a[4];
int f(void) { int *e = &a[4]; return a[3] + (e != 0); }
```

### Violation (must be reported): shift count equal to the width of int

`main.c`

```c
int f(int a) { return a << 32; } /*@V*/
```

### Compliant (must stay silent): shift count one less than the width

`main.c`

```c
int f(int a) { return a << 31; }
```

### Violation (must be reported): negative shift count

`main.c`

```c
int f(int a) { return a >> -1; } /*@V*/
```

### Compliant (must stay silent): offsetof idiom through a null pointer is not a dereference

`main.c`

```c
#include <stddef.h>
struct s { int a; int b; };
unsigned long off = offsetof(struct s, b);
```

### Violation (must be reported): member access through a null pointer constant

`main.c`

```c
struct s { int a; };
int f(void) { return ((struct s *)0)->a; } /*@V*/
```

### Compliant (must stay silent): division where zero is excluded by the type of the divisor constant

`main.c`

```c
int f(int a) { return a / 2 + a % 3; }
```

### Compliant (must stay silent): two-dimensional index in range

`main.c`

```c
int m[3][4];
int f(void) { return m[2][3]; }
```

### Violation (must be reported): second dimension index out of range

`main.c`

```c
int m[3][4];
int f(void) { return m[1][4]; } /*@V*/
```

### Compliant (must stay silent): index into a pointer parameter is unknown

`main.c`

```c
int f(int *p) { return p[100]; }
```

### Compliant (must stay silent): zero literal as the dividend

`main.c`

```c
int f(int a) { return 0 / (a | 1); }
```

### Violation (must be reported): null dereference through the NULL macro

`main.c`

```c
#include <stddef.h>
int f(void) { return *(int *)NULL; } /*@V*/
```

### Compliant (must stay silent): division by a macro that is non-zero

`main.c`

```c
#define SCALE 4
int f(int a) { return a / SCALE; }
```

### Violation (must be reported): division by a macro that is zero

`main.c`

```c
#define SCALE 0
int f(int a) { return a / SCALE; } /*@V*/
```

### Violation (must be reported): constant index into a string literal past its end

`main.c`

```c
char f(void) { return "ab"[3]; } /*@V*/
```

### Compliant (must stay silent): index at the terminator of a string literal

`main.c`

```c
char f(void) { return "ab"[2]; }
```

