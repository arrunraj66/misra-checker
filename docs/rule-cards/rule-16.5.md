# Rule 16.5

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Switch statements

## What the detector reports

Reports a switch whose default clause is neither the first nor the last clause.

Detection contract: Report a switch whose default clause is neither the first nor the last clause.

## Known limits

A switch without default is Rule 16.4.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | default last | pass |
| Compliant (must stay silent) | default first | pass |
| Violation (must be reported) | default in the middle | pass |

## Case sources

### Compliant (must stay silent): default last

`main.c`

```c
int f(int v) { switch (v) { case 1: return 1; case 2: return 2; default: return 0; } }
```

### Compliant (must stay silent): default first

`main.c`

```c
int g(int v) { switch (v) { default: return 0; case 1: return 1; case 2: return 2; } }
```

### Violation (must be reported): default in the middle

`main.c`

```c
int f(int v)
{
    switch (v) /*@V*/
    {
        case 1: return 1;
        default: return 0;
        case 2: return 2;
    }
}
```

