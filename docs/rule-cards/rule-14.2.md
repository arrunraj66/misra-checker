# Rule 14.2

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Control statement expressions

## What the detector reports

Subset: reports a for statement that does not have exactly one loop counter that its third clause modifies, its second clause tests and its body leaves alone (the form for(;;) is accepted).

Detection contract: Subset: report a for statement that lacks a single loop counter modified only by its third clause, tested by its second clause, and untouched by the body (the no-clause for(;;) form is exempt).

## Known limits

Only simple counters in the clauses are recognized; counters hidden in function calls are not.

## Tested cases

10 of 10 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | the standard counting loop | pass |
| Compliant (must stay silent) | the infinite loop | pass |
| Compliant (must stay silent) | a declaration-free counter in the first clause and a step | pass |
| Violation (must be reported) | the body modifies the counter | pass |
| Violation (must be reported) | the second clause tests something else than the counter | pass |
| Violation (must be reported) | no third clause | pass |
| Violation (must be reported) | two counters modified in the third clause | pass |
| Compliant (must stay silent) | a step other than one | pass |
| Compliant (must stay silent) | a pointer walking a list | pass |
| Compliant (must stay silent) | a counting down loop with the counter only in the clauses | pass |

## Case sources

### Compliant (must stay silent): the standard counting loop

`main.c`

```c
void f(int n) { int i; for (i = 0; i < n; i++) { } }
```

### Compliant (must stay silent): the infinite loop

`main.c`

```c
void g(void) { for (;;) { break; } }
```

### Compliant (must stay silent): a declaration-free counter in the first clause and a step

`main.c`

```c
void h(int n) { int i; for (i = n; i > 0; i--) { } }
```

### Violation (must be reported): the body modifies the counter

`main.c`

```c
void f(int n)
{
    int i;
    for (i = 0; i < n; i++) /*@V*/
    {
        i++;
    }
}
```

### Violation (must be reported): the second clause tests something else than the counter

`main.c`

```c
void g(int n, int flag)
{
    int i;
    for (i = 0; flag != 0; i++) { flag--; } /*@V*/
    (void)n;
}
```

### Violation (must be reported): no third clause

`main.c`

```c
void h(int n)
{
    int i = 0;
    for (; i < n; ) { i++; } /*@V*/
}
```

### Violation (must be reported): two counters modified in the third clause

`main.c`

```c
void k(int n)
{
    int i, j;
    for (i = 0, j = 0; i < n; i++, j++) { } /*@V*/
}
```

### Compliant (must stay silent): a step other than one

`main.c`

```c
void f(int n) { int i; for (i = 0; i < n; i += 2) { } }
```

### Compliant (must stay silent): a pointer walking a list

`main.c`

```c
struct node { struct node *next; };
void f(struct node *head)
{
    struct node *p;
    for (p = head; p != 0; p = p->next) { }
}
```

### Compliant (must stay silent): a counting down loop with the counter only in the clauses

`main.c`

```c
void f(int n) { int i; for (i = n; i != 0; i--) { } }
```

