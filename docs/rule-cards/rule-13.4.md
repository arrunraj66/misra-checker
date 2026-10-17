# Rule 13.4

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Side effects and evaluation order

## What the detector reports

Reports an assignment whose result is used by an enclosing expression or condition.

Detection contract: Report an assignment whose result is used by an enclosing expression or condition.

## Known limits

Assignments that stand alone as expression statements are accepted; the initialization of a declaration is not an assignment expression.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | assignment statements | pass |
| Compliant (must stay silent) | a for statement clause that assigns | pass |
| Violation (must be reported) | assignment used as a condition | pass |
| Violation (must be reported) | chained assignment | pass |
| Violation (must be reported) | assignment inside an argument | pass |

## Case sources

### Compliant (must stay silent): assignment statements

`main.c`

```c
void f(void)
{
    int a;
    int b;
    a = 1;
    b = a;
    (void)b;
}
```

### Compliant (must stay silent): a for statement clause that assigns

`main.c`

```c
void g(void) { int i; for (i = 0; i < 3; i++) { } }
```

### Violation (must be reported): assignment used as a condition

`main.c`

```c
int f(int v)
{
    int x;
    if ((x = v) != 0) { return x; } /*@V*/
    return 0;
}
```

### Violation (must be reported): chained assignment

`main.c`

```c
void g(void)
{
    int a, b;
    a = b = 0; /*@V*/
    (void)a;
}
```

### Violation (must be reported): assignment inside an argument

`main.c`

```c
void use(int);
void h(int v)
{
    int x;
    use(x = v); /*@V*/
}
```

