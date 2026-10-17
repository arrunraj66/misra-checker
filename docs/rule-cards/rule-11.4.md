# Rule 11.4

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports an explicit cast between a pointer and an integer type (a null constant is exempt).

Detection contract: Report an explicit cast between a pointer and an integer type (null constants excluded).

## Known limits

Only explicit casts are examined.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | null pointer constants | pass |
| Compliant (must stay silent) | no cast | pass |
| Violation (must be reported) | integer address cast to a pointer | pass |
| Violation (must be reported) | pointer cast to an integer | pass |
| Violation (must be reported) | a memory-mapped register address | pass |
| Compliant (must stay silent) | a cast between pointers is not an integer cast | pass |
| Violation (must be reported) | a cast of an integer constant to a pointer inside a function | pass |

## Case sources

### Compliant (must stay silent): null pointer constants

`main.c`

```c
int *p = (int *)0;
void *q = (void *)0;
```

### Compliant (must stay silent): no cast

`main.c`

```c
int *r = 0;
```

### Violation (must be reported): integer address cast to a pointer

`main.c`

```c
int *p = (int *)0x1000; /*@V*/
```

### Violation (must be reported): pointer cast to an integer

`main.c`

```c
unsigned long f(int *p) { return (unsigned long)p; } /*@V*/
```

### Violation (must be reported): a memory-mapped register address

`main.c`

```c
#define REG (*(volatile unsigned int *)0x40021000)
unsigned int f(void) { return REG; } /*@V*/
```

### Compliant (must stay silent): a cast between pointers is not an integer cast

`main.c`

```c
int *a(void *p);
```

### Violation (must be reported): a cast of an integer constant to a pointer inside a function

`main.c`

```c
int f(void)
{
    int *p = (int *)4096; /*@V*/
    return *p;
}
```

