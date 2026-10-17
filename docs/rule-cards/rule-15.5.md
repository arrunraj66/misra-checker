# Rule 15.5

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Control flow

## What the detector reports

Reports a function with more than one return statement, or whose single return is not the last statement of the body.

Detection contract: Report a function with more than one return statement, or whose single return is not the last statement of the body.

## Known limits

Functions of void type with a bare return are examined the same way.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a single return at the end | pass |
| Compliant (must stay silent) | a function without any return | pass |
| Violation (must be reported) | two returns | pass |
| Violation (must be reported) | a return that is not the last statement | pass |
| Violation (must be reported) | an early return in a void function | pass |

## Case sources

### Compliant (must stay silent): a single return at the end

`main.c`

```c
int f(int x) { int r = x; if (x > 0) { r = 1; } return r; }
```

### Compliant (must stay silent): a function without any return

`main.c`

```c
void g(void) { }
```

### Violation (must be reported): two returns

`main.c`

```c
int f(int x) /*@V*/
{
    if (x > 0) { return 1; }
    return 0;
}
```

### Violation (must be reported): a return that is not the last statement

`main.c`

```c
int g(int x) /*@V*/
{
    return x;
    x = 1;
}
```

### Violation (must be reported): an early return in a void function

`main.c`

```c
void h(int x) /*@V*/
{
    if (x) { return; }
    x = 1;
}
```

