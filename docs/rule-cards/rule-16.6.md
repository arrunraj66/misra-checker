# Rule 16.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Switch statements

## What the detector reports

Reports a switch statement with fewer than two clauses.

Detection contract: Report a switch statement with fewer than two clauses.

## Known limits

A clause with several stacked labels counts once.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | two clauses | pass |
| Violation (must be reported) | only a default clause | pass |
| Violation (must be reported) | a single case clause | pass |
| Violation (must be reported) | two labels stacked on one clause only | pass |
| Compliant (must stay silent) | a case clause and a default clause | pass |
| Violation (must be reported) | only a default and a stacked default label | pass |
| Compliant (must stay silent) | three clauses | pass |

## Case sources

### Compliant (must stay silent): two clauses

`main.c`

```c
int f(int v) { switch (v) { case 1: return 1; default: return 0; } }
```

### Violation (must be reported): only a default clause

`main.c`

```c
int f(int v)
{
    switch (v) /*@V*/
    {
        default: return v;
    }
}
```

### Violation (must be reported): a single case clause

`main.c`

```c
int g(int v)
{
    switch (v) /*@V*/
    {
        case 1: return 1;
    }
    return 0;
}
```

### Violation (must be reported): two labels stacked on one clause only

`main.c`

```c
void f(int v)
{
    switch (v) /*@V*/
    {
        case 1:
        case 2:
            v = 0;
            break;
    }
}
```

### Compliant (must stay silent): a case clause and a default clause

`main.c`

```c
void f(int v)
{
    switch (v)
    {
        case 1: v = 0; break;
        default: break;
    }
}
```

### Violation (must be reported): only a default and a stacked default label

`main.c`

```c
void f(int v)
{
    switch (v) /*@V*/
    {
        default:
            v = 1;
            break;
    }
}
```

### Compliant (must stay silent): three clauses

`main.c`

```c
void f(int v)
{
    switch (v)
    {
        case 1: break;
        case 2: break;
        default: break;
    }
}
```

