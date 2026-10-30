# Rule 15.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Control flow

## What the detector reports

Reports an if, else, loop or switch body that is not a compound statement (an else-if chain is accepted).

Detection contract: Report an if, else, loop or switch body that is not a compound statement (else-if chains are allowed).

## Known limits

Only the bodies are examined, not the layout.

## Tested cases

12 of 12 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | bodies in braces | pass |
| Compliant (must stay silent) | an else-if chain | pass |
| Violation (must be reported) | an if body without braces | pass |
| Violation (must be reported) | a loop body without braces | pass |
| Violation (must be reported) | an else body without braces | pass |
| Violation (must be reported) | an empty statement as a loop body | pass |
| Compliant (must stay silent) | macro supplies the braces | pass |
| Compliant (must stay silent) | else block containing a nested if | pass |
| Violation (must be reported) | switch body statement without braces is invalid C; a case body is fine | pass |
| Violation (must be reported) | a do-while body without braces | pass |
| Violation (must be reported) | an infinite for with a single statement | pass |
| Compliant (must stay silent) | an else-if chain with braces everywhere | pass |

## Case sources

### Compliant (must stay silent): bodies in braces

`main.c`

```c
void f(int x) { if (x) { x = 1; } else { x = 2; } while (x > 0) { x--; } }
```

### Compliant (must stay silent): an else-if chain

`main.c`

```c
int g(int x) { if (x == 1) { return 1; } else if (x == 2) { return 2; } else { return 3; } }
```

### Violation (must be reported): an if body without braces

`main.c`

```c
void f(int x)
{
    if (x) x = 1; /*@V*/
}
```

### Violation (must be reported): a loop body without braces

`main.c`

```c
void g(int n)
{
    while (n > 0) n--; /*@V*/
}
```

### Violation (must be reported): an else body without braces

`main.c`

```c
void h(int x)
{
    if (x) { x = 1; } else x = 2; /*@V*/
}
```

### Violation (must be reported): an empty statement as a loop body

`main.c`

```c
void f(int n) { while (n--) ; } /*@V*/
```

### Compliant (must stay silent): macro supplies the braces

`main.c`

```c
#define DO_TWICE(x) { x; x; }
void g(void);
void f(int a) { if (a) DO_TWICE(g()) }
```

### Compliant (must stay silent): else block containing a nested if

`main.c`

```c
void f(int a, int b)
{
    if (a) { b = 1; } else { if (b) { b = 2; } }
}
```

### Violation (must be reported): switch body statement without braces is invalid C; a case body is fine

`main.c`

```c
void f(int a)
{
    for (a = 0; a < 2; a++) a++; /*@V*/
}
```

### Violation (must be reported): a do-while body without braces

`main.c`

```c
void f(int x)
{
    do x--; while (x > 0); /*@V*/
}
```

### Violation (must be reported): an infinite for with a single statement

`main.c`

```c
void g(int *p)
{
    for (;;) (*p)++; /*@V*/
}
```

### Compliant (must stay silent): an else-if chain with braces everywhere

`main.c`

```c
int f(int x) { if (x > 1) { return 1; } else if (x > 0) { return 0; } else { return -1; } }
```

