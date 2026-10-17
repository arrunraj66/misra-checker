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

7 of 9 pass.

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

