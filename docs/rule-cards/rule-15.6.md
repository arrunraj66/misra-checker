# Rule 15.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Control flow

## What the detector reports

Reports an if, else, loop or switch body that is not a compound statement (an else-if chain is accepted).

Detection contract: Report an if, else, loop or switch body that is not a compound statement (else-if chains are allowed).

## Known limits

Only the bodies are examined, not the layout.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | bodies in braces | pass |
| Compliant (must stay silent) | an else-if chain | pass |
| Violation (must be reported) | an if body without braces | pass |
| Violation (must be reported) | a loop body without braces | pass |
| Violation (must be reported) | an else body without braces | pass |
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

