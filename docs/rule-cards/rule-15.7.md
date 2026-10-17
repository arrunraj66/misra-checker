# Rule 15.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Control flow

## What the detector reports

Reports an if / else-if chain that does not end in a plain else.

Detection contract: Report an if/else-if chain that does not end in a plain else.

## Known limits

A lone if without else-if is accepted.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a lone if | pass |
| Compliant (must stay silent) | a chain ending in else | pass |
| Violation (must be reported) | a chain without a final else | pass |
| Violation (must be reported) | a longer chain without a final else | pass |

## Case sources

### Compliant (must stay silent): a lone if

`main.c`

```c
void f(int x) { if (x) { x = 1; } }
```

### Compliant (must stay silent): a chain ending in else

`main.c`

```c
int g(int x) { if (x == 1) { return 1; } else if (x == 2) { return 2; } else { return 3; } }
```

### Violation (must be reported): a chain without a final else

`main.c`

```c
int f(int x)
{
    if (x == 1) { return 1; } /*@V*/
    else if (x == 2) { return 2; }
    return 0;
}
```

### Violation (must be reported): a longer chain without a final else

`main.c`

```c
void g(int x)
{
    if (x == 1) { x = 0; } /*@V*/
    else if (x == 2) { x = 1; }
    else if (x == 3) { x = 2; }
}
```

