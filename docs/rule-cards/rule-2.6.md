# Rule 2.6

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Unused and unreachable code

## What the detector reports

Reports a label in a function body that no goto statement or label-address expression refers to.

Detection contract: Report a label in a function body that no goto statement or label-address expression references.

## Known limits

Labels inside macros that are expanded elsewhere are judged where they are written.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | label that is a goto target | pass |
| Violation (must be reported) | unused label | pass |
| Violation (must be reported) | label that only a different function's goto uses (labels are per function) | pass |

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

