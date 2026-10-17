# Rule 14.1

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Control statement expressions

## What the detector reports

Reports a for loop whose increment expression modifies a floating-point variable (a floating-point loop counter).

Detection contract: Report a for loop whose increment expression modifies a floating-point variable.

## Known limits

Only for loops are examined, not while loops that use a float.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | integer counter | pass |
| Compliant (must stay silent) | a floating-point value updated inside the body | pass |
| Violation (must be reported) | float counter | pass |
| Violation (must be reported) | double counter | pass |

## Case sources

### Compliant (must stay silent): integer counter

`main.c`

```c
void f(void) { int i; for (i = 0; i < 10; i++) { } }
```

### Compliant (must stay silent): a floating-point value updated inside the body

`main.c`

```c
double g(int n)
{
    double acc = 0.0;
    int i;
    for (i = 0; i < n; i++) { acc += 0.1; }
    return acc;
}
```

### Violation (must be reported): float counter

`main.c`

```c
void f(void)
{
    float x;
    for (x = 0.0f; x < 1.0f; x += 0.1f) { } /*@V*/
}
```

### Violation (must be reported): double counter

`main.c`

```c
void g(void)
{
    double d;
    for (d = 0.0; d < 1.0; d = d + 0.25) { } /*@V*/
}
```

