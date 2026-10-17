# Rule 16.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Switch statements

## What the detector reports

Reports a switch statement with fewer than two clauses.

Detection contract: Report a switch statement with fewer than two clauses.

## Known limits

A clause with several stacked labels counts once.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | two clauses | pass |
| Violation (must be reported) | only a default clause | pass |
| Violation (must be reported) | a single case clause | pass |

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

