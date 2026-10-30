# Rule 15.1

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Control flow

## What the detector reports

Reports every explicit goto statement written in the main file.

Detection contract: Report every explicit goto statement written in the

## Known limits

A goto produced by a macro expansion is reported at the use.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no goto | pass |
| Compliant (must stay silent) | a label is not a goto | pass |
| Violation (must be reported) | a forward goto | pass |
| Violation (must be reported) | a backward goto | pass |
| Violation (must be reported) | goto in a nested block | pass |
| Compliant (must stay silent) | a switch label and a case are not gotos | pass |
| Violation (must be reported) | a goto forward | pass |
| Compliant (must stay silent) | no goto | pass |

## Case sources

### Compliant (must stay silent): no goto

`main.c`

```c
int f(int x) { return x + 1; }
```

### Compliant (must stay silent): a label is not a goto

`main.c`

```c
void g(void)
{
    int i = 0;
again:
    i++;
    (void)i;
}
```

### Violation (must be reported): a forward goto

`main.c`

```c
int f(int x)
{
    if (x) { goto done; } /*@V*/
    x = 1;
done:
    return x;
}
```

### Violation (must be reported): a backward goto

`main.c`

```c
void g(void)
{
    int i = 0;
again:
    i++;
    if (i < 3) { goto again; } /*@V*/
}
```

### Violation (must be reported): goto in a nested block

`main.c`

```c
void f(int a)
{
    if (a) { goto end; } /*@V*/
end:
    a = 0;
}
```

### Compliant (must stay silent): a switch label and a case are not gotos

`main.c`

```c
int f(int v) { switch (v) { case 1: return 1; default: return 0; } }
```

### Violation (must be reported): a goto forward

`main.c`

```c
void f(int x)
{
    if (x) { goto end; } /*@V*/
    x = 1;
end:
    x = 2;
}
```

### Compliant (must stay silent): no goto

`main.c`

```c
void f(int x)
{
    if (x) { x = 1; }
}
```

