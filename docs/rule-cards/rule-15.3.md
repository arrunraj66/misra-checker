# Rule 15.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Control flow

## What the detector reports

Reports a goto whose label is not declared in the same block as the goto or in an enclosing block.

Detection contract: For every resolved goto in a successfully parsed

## Known limits

Needs a successfully parsed translation unit with the label resolved.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | label in the same block | pass |
| Compliant (must stay silent) | label in an enclosing block | pass |
| Violation (must be reported) | a jump into a nested block | pass |
| Violation (must be reported) | a jump into a sibling block | pass |

## Case sources

### Compliant (must stay silent): label in the same block

`main.c`

```c
void f(int x)
{
    if (x) { goto end; }
    x = 1;
end:
    return;
}
```

### Compliant (must stay silent): label in an enclosing block

`main.c`

```c
void g(int x)
{
    {
        if (x) { goto end; }
    }
end:
    return;
}
```

### Violation (must be reported): a jump into a nested block

`main.c`

```c
void f(int x)
{
    goto inside; /*@V*/
    {
inside:
        x = 1;
    }
}
```

### Violation (must be reported): a jump into a sibling block

`main.c`

```c
void g(int x)
{
    if (x)
    {
        goto other; /*@V*/
    }
    else
    {
other:
        x = 2;
    }
}
```

