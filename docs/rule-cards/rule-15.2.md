# Rule 15.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Control flow

## What the detector reports

Reports a goto whose label is declared earlier in the function (a backward jump).

Detection contract: For every resolved goto in a successfully parsed

## Known limits

Needs a successfully parsed translation unit with the label resolved.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | only forward jumps | pass |
| Compliant (must stay silent) | no goto at all | pass |
| Violation (must be reported) | a backward jump | pass |
| Violation (must be reported) | a backward jump out of a nested block | pass |

## Case sources

### Compliant (must stay silent): only forward jumps

`main.c`

```c
int f(int x)
{
    if (x) { goto done; }
    x = 1;
done:
    return x;
}
```

### Compliant (must stay silent): no goto at all

`main.c`

```c
int g(int x) { return x; }
```

### Violation (must be reported): a backward jump

`main.c`

```c
void f(void)
{
    int i = 0;
again:
    i++;
    if (i < 3) { goto again; } /*@V*/
}
```

### Violation (must be reported): a backward jump out of a nested block

`main.c`

```c
void g(int n)
{
top:
    n--;
    if (n > 0)
    {
        goto top; /*@V*/
    }
}
```

