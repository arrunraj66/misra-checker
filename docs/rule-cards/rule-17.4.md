# Rule 17.4

**Category:** Mandatory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Functions

## What the detector reports

Reports a return without a value in a non-void function, and a non-void function that has a reachable path leaving the body without returning a value (main is exempt).

Detection contract: Report a return without a value in a non-void function, or a non-void function with a reachable control-flow path that leaves the body without returning a value.

## Known limits

Functions that compile errors have made invalid, and functions whose body was emptied by an error, are not examined.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | every path returns a value | pass |
| Compliant (must stay silent) | a void function | pass |
| Compliant (must stay silent) | main is exempt | pass |
| Compliant (must stay silent) | a function that never returns | pass |
| Violation (must be reported) | one path has no return | pass |
| Violation (must be reported) | a switch whose clauses do not all return | pass |
| Violation (must be reported) | falls off the end after a loop | pass |

## Case sources

### Compliant (must stay silent): every path returns a value

`main.c`

```c
int f(int x) { if (x) { return 1; } return 0; }
```

### Compliant (must stay silent): a void function

`main.c`

```c
void g(int x) { if (x) { return; } }
```

### Compliant (must stay silent): main is exempt

`main.c`

```c
int main(void) { }
```

### Compliant (must stay silent): a function that never returns

`main.c`

```c
void h(void) { for (;;) { } }
```

### Violation (must be reported): one path has no return

`main.c`

```c
int f(int x) /*@V*/
{
    if (x) { return 1; }
}
```

### Violation (must be reported): a switch whose clauses do not all return

`main.c`

```c
int g(int v) /*@V*/
{
    switch (v)
    {
        case 1: return 1;
        default: break;
    }
}
```

### Violation (must be reported): falls off the end after a loop

`main.c`

```c
int k(int n) /*@V*/
{
    while (n > 0) { n--; }
}
```

