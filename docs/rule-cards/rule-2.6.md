# Rule 2.6

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Unused and unreachable code

## What the detector reports

Reports a label in a function body that no goto statement or label-address expression refers to.

Detection contract: Report a label in a function body that no goto statement or label-address expression references.

## Known limits

Labels inside macros that are expanded elsewhere are judged where they are written.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | label that is a goto target | pass |
| Violation (must be reported) | unused label | pass |
| Violation (must be reported) | label that only a different function's goto uses (labels are per function) | pass |
| Compliant (must stay silent) | backward goto to a label | pass |
| Compliant (must stay silent) | goto from a nested block | pass |
| Violation (must be reported) | two labels where only one is used | pass |
| Compliant (must stay silent) | labels in a switch (case) are not goto labels | pass |

## Case sources

### Compliant (must stay silent): label that is a goto target

`main.c`

```c
void f(int v)
{
    if (v) { goto done; }
    v = 1;
done:
    return;
}
```

### Violation (must be reported): unused label

`main.c`

```c
void f(void)
{
unused_label: /*@V*/
    return;
}
```

### Violation (must be reported): label that only a different function's goto uses (labels are per function)

`main.c`

```c
void g(void)
{
    goto end;
end:
    return;
}
void h(void)
{
end: /*@V*/
    return;
}
```

### Compliant (must stay silent): backward goto to a label

`main.c`

```c
void f(int n)
{
again:
    n--;
    if (n > 0) { goto again; }
}
```

### Compliant (must stay silent): goto from a nested block

`main.c`

```c
void f(int n)
{
    while (n)
    {
        if (n == 2) { goto out; }
        n--;
    }
out:
    n = 0;
}
```

### Violation (must be reported): two labels where only one is used

`main.c`

```c
void f(int n)
{
    if (n) { goto used; }
unused: /*@V*/
    n = 1;
used:
    n = 2;
}
```

### Compliant (must stay silent): labels in a switch (case) are not goto labels

`main.c`

```c
int f(int v)
{
    switch (v) { case 1: return 1; default: return 0; }
}
```

