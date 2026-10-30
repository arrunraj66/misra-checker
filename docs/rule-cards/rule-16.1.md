# Rule 16.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Switch statements

## What the detector reports

Reports any violation of the structure rules for switch statements (labels at the top level of the body, a break per clause, default placement, clause count, and an essentially non-Boolean controlling expression).

Detection contract: Report any violation of the switch-structure constituents (labels at top level, break per clause, default placement, clause count, non-Boolean controlling expression).

## Known limits

A summary rule that overlaps 16.2 to 16.7; those individual rules are reported as well.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a well formed switch | pass |
| Violation (must be reported) | a clause that falls through | pass |
| Violation (must be reported) | a switch without default | pass |
| Compliant (must stay silent) | default listed first | pass |
| Violation (must be reported) | switch with a single clause | pass |

## Case sources

### Compliant (must stay silent): a well formed switch

`main.c`

```c
int f(int v)
{
    int r;
    switch (v)
    {
        case 1:
            r = 10;
            break;
        case 2:
            r = 20;
            break;
        default:
            r = 0;
            break;
    }
    return r;
}
```

### Violation (must be reported): a clause that falls through

`main.c`

```c
int f(int v)
{
    int r = 0;
    switch (v)
    {
        case 1: /*@V*/
            r = 1;
        case 2:
            r = 2;
            break;
        default:
            break;
    }
    return r;
}
```

### Violation (must be reported): a switch without default

`main.c`

```c
int g(int v)
{
    int r = 0;
    switch (v) /*@V*/
    {
        case 1:
            r = 1;
            break;
        case 2:
            r = 2;
            break;
    }
    return r;
}
```

### Compliant (must stay silent): default listed first

`main.c`

```c
void f(int v)
{
    switch (v)
    {
        default:
            break;
        case 1:
            v = 2;
            break;
    }
}
```

### Violation (must be reported): switch with a single clause

`main.c`

```c
void f(int v)
{
    switch (v)
    {
        default: v = 1; break;
    }
}
```

