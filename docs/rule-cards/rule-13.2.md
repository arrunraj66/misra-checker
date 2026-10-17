# Rule 13.2

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Side effects and evaluation order

## What the detector reports

Subset: reports a scalar variable that is modified and also read or modified without an intervening sequence point in one full expression.

Detection contract: Subset: report a scalar variable that is modified and also read or modified without a sequence point in one full expression.

## Known limits

Only plain scalar variables are tracked; calls that modify through pointers and array elements with computed indexes are not.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a modification in its own statement | pass |
| Compliant (must stay silent) | separated by sequence points | pass |
| Violation (must be reported) | read and modified in one expression | pass |
| Violation (must be reported) | two modifications of the same variable | pass |
| Violation (must be reported) | index uses a variable that the same expression modifies | pass |
| Compliant (must stay silent) | different variables in one expression | pass |
| Compliant (must stay silent) | function arguments that do not conflict | pass |
| Compliant (must stay silent) | a variable modified in one statement and read in the next | pass |

## Case sources

### Compliant (must stay silent): a modification in its own statement

`main.c`

```c
int f(int i) { i = i + 1; return i; }
```

### Compliant (must stay silent): separated by sequence points

`main.c`

```c
int g(int a, int b) { return (a > 0) && (b++ > 0); }
```

### Violation (must be reported): read and modified in one expression

`main.c`

```c
int f(int i)
{
    i = i++ + 1; /*@V*/
    return i;
}
```

### Violation (must be reported): two modifications of the same variable

`main.c`

```c
int g(int i)
{
    int r = i++ + i++; /*@V*/
    return r;
}
```

### Violation (must be reported): index uses a variable that the same expression modifies

`main.c`

```c
int h(int i, int *a)
{
    a[i] = i++; /*@V*/
    return a[0];
}
```

### Compliant (must stay silent): different variables in one expression

`main.c`

```c
int f(int i, int j) { return i + j++; }
```

### Compliant (must stay silent): function arguments that do not conflict

`main.c`

```c
int add(int a, int b);
int f(int i, int j) { return add(i, j + 1); }
```

### Compliant (must stay silent): a variable modified in one statement and read in the next

`main.c`

```c
int f(int i)
{
    i++;
    return i;
}
```

