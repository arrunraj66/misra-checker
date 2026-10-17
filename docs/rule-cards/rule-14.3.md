# Rule 14.3

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Control statement expressions

## What the detector reports

Subset: reports if, do, conditional-operator and false while/for conditions that evaluate to a constant outside macro expansions.

Detection contract: Subset: report if, do, conditional-operator, and false while/for conditions that evaluate to a constant outside macro expansions.

## Known limits

while(1) and for(;;) infinite loops are accepted. Conditions in macros are skipped. Only constants the compiler folds are found.

- **False negative:** a constant that is folded only through a configuration macro (conditions from macro expansions are skipped) (documented limit)

## Tested cases

5 of 6 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | variable conditions | pass |
| Compliant (must stay silent) | infinite loops and the do-while-zero idiom inside a macro | pass |
| Violation (must be reported) | a constant condition in an if | pass |
| Violation (must be reported) | a constant conditional operator | pass |
| Violation (must be reported) | a false loop condition | pass |
| Known false negative (not detected yet) | a constant that is folded only through a configuration macro (conditions from macro expansions are skipped) | documented limit |

## Case sources

### Compliant (must stay silent): variable conditions

`main.c`

```c
int f(int x) { if (x > 0) { return 1; } return 0; }
```

### Compliant (must stay silent): infinite loops and the do-while-zero idiom inside a macro

`main.c`

```c
#define ONCE(stmt) do { stmt; } while (0)
void g(int *p) { for (;;) { break; } ONCE(*p = 1); }
```

### Violation (must be reported): a constant condition in an if

`main.c`

```c
int f(void)
{
    if (1) { return 1; } /*@V*/
    return 0;
}
```

### Violation (must be reported): a constant conditional operator

`main.c`

```c
int g(void) { return 0 ? 1 : 2; } /*@V*/
```

### Violation (must be reported): a false loop condition

`main.c`

```c
void h(void)
{
    while (0) { } /*@V*/
}
```

### Known false negative (not detected yet): a constant that is folded only through a configuration macro (conditions from macro expansions are skipped)

`main.c`

```c
#define LIMIT 4
int k(void)
{
    if (LIMIT > 10) { return 1; } /*@V*/
    return 0;
}
```

