# Rule 13.4

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Side effects and evaluation order

## What the detector reports

Reports an assignment whose result is used by an enclosing expression or condition.

Detection contract: Report an assignment whose result is used by an enclosing expression or condition.

## Known limits

Assignments that stand alone as expression statements are accepted; the initialization of a declaration is not an assignment expression.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | assignment statements | pass |
| Compliant (must stay silent) | a for statement clause that assigns | pass |
| Violation (must be reported) | assignment used as a condition | pass |
| Violation (must be reported) | chained assignment | pass |
| Violation (must be reported) | assignment inside an argument | pass |
| Violation (must be reported) | assignment returned | pass |
| Violation (must be reported) | assignment in a loop condition | pass |
| Compliant (must stay silent) | compound assignment as a statement | pass |
| Compliant (must stay silent) | assignments in separate for clauses | pass |

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

### Violation (must be reported): assignment returned

`main.c`

```c
int f(int a) { int b; return b = a; } /*@V*/
```

### Violation (must be reported): assignment in a loop condition

`main.c`

```c
int f(int *p)
{
    int c;
    int n = 0;
    while ((c = *p++) != 0) { n += c; } /*@V*/
    return n;
}
```

### Compliant (must stay silent): compound assignment as a statement

`main.c`

```c
void f(int a) { a += 2; (void)a; }
```

### Compliant (must stay silent): assignments in separate for clauses

`main.c`

```c
void f(int n)
{
    int i, j;
    for (i = 0, j = n; i < j; i++) { }
}
```

