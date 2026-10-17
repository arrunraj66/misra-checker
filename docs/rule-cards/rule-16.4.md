# Rule 16.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Switch statements

## What the detector reports

Reports a switch statement without a default label.

Detection contract: Report a switch statement without a default label.

## Known limits

Only the presence of a default label is examined.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a switch with default | pass |
| Violation (must be reported) | no default | pass |
| Violation (must be reported) | only a case label | pass |

## Case sources

### Compliant (must stay silent): a switch with default

`main.c`

```c
int f(int v) { switch (v) { case 1: return 1; default: return 0; } }
```

### Violation (must be reported): no default

`main.c`

```c
int f(int v)
{
    int r = 0;
    switch (v) /*@V*/
    {
        case 1: r = 1; break;
        case 2: r = 2; break;
    }
    return r;
}
```

### Violation (must be reported): only a case label

`main.c`

```c
int g(int v)
{
    switch (v) /*@V*/
    {
        case 1:
            return 1;
    }
    return 0;
}
```

