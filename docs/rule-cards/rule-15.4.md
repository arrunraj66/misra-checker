# Rule 15.4

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Control flow

## What the detector reports

Reports a for, while or do loop that has more than one break or goto leaving it.

Detection contract: For every for, while and do loop written in the primary

## Known limits

Only the loops written in the main file are examined. A break in a nested switch belongs to the switch.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a loop with one break | pass |
| Compliant (must stay silent) | no break | pass |
| Compliant (must stay silent) | a break that belongs to an inner switch | pass |
| Violation (must be reported) | two breaks | pass |
| Violation (must be reported) | a break and a goto out of the loop | pass |
| Compliant (must stay silent) | one break in each of two nested loops | pass |
| Compliant (must stay silent) | continue does not count | pass |
| Violation (must be reported) | two breaks in a while loop | pass |
| Compliant (must stay silent) | breaks inside a switch inside a loop leave the switch | pass |

## Case sources

### Compliant (must stay silent): a loop with one break

`main.c`

```c
int f(int n)
{
    int i;
    for (i = 0; i < n; i++) { if (i == 5) { break; } }
    return i;
}
```

### Compliant (must stay silent): no break

`main.c`

```c
void g(int n) { while (n > 0) { n--; } }
```

### Compliant (must stay silent): a break that belongs to an inner switch

`main.c`

```c
void h(int n)
{
    while (n > 0)
    {
        switch (n) { case 1: break; default: break; }
        n--;
    }
}
```

### Violation (must be reported): two breaks

`main.c`

```c
int f(int n)
{
    int i;
    for (i = 0; i < n; i++) /*@V*/
    {
        if (i == 5) { break; }
        if (i == 7) { break; }
    }
    return i;
}
```

### Violation (must be reported): a break and a goto out of the loop

`main.c`

```c
int g(int n)
{
    int i;
    for (i = 0; i < n; i++) /*@V*/
    {
        if (i == 5) { break; }
        if (i == 7) { goto out; }
    }
out:
    return i;
}
```

### Compliant (must stay silent): one break in each of two nested loops

`main.c`

```c
void f(int n, int m)
{
    int i, j;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < m; j++)
        {
            if (j == 3) { break; }
        }
        if (i == 2) { break; }
    }
}
```

### Compliant (must stay silent): continue does not count

`main.c`

```c
void f(int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (i == 1) { continue; }
        if (i == 2) { continue; }
    }
}
```

### Violation (must be reported): two breaks in a while loop

`main.c`

```c
void f(int n)
{
    while (n) /*@V*/
    {
        if (n == 1) { break; }
        if (n == 2) { break; }
        n--;
    }
}
```

### Compliant (must stay silent): breaks inside a switch inside a loop leave the switch

`main.c`

```c
void f(int n)
{
    while (n)
    {
        switch (n)
        {
            case 1: n = 0; break;
            default: n--; break;
        }
    }
}
```

