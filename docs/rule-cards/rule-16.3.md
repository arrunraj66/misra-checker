# Rule 16.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Switch statements

## What the detector reports

Strict reading: reports a switch clause whose last statement is not a break (or a block ending in break).

Detection contract: Strict reading: report a switch clause whose last statement is not a break (or a block ending in break).

## Known limits

Consecutive empty labels sharing a clause are accepted. A clause that ends in return or goto is reported under the strict reading.

## Tested cases

10 of 10 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | every clause ends in break | pass |
| Compliant (must stay silent) | stacked labels | pass |
| Violation (must be reported) | a clause that falls through | pass |
| Violation (must be reported) | a clause that ends in return (strict reading) | pass |
| Violation (must be reported) | a clause ending with a conditional break only | pass |
| Violation (must be reported) | the last clause without break | pass |
| Compliant (must stay silent) | stacked labels sharing a clause | pass |
| Violation (must be reported) | a clause ending with goto | pass |
| Compliant (must stay silent) | a clause that is a block ending in break | pass |
| Violation (must be reported) | a clause that ends in continue inside a loop | pass |

## Case sources

### Compliant (must stay silent): every clause ends in break

`main.c`

```c
int f(int v)
{
    int r;
    switch (v)
    {
        case 1: r = 1; break;
        case 2: r = 2; break;
        default: r = 0; break;
    }
    return r;
}
```

### Compliant (must stay silent): stacked labels

`main.c`

```c
int g(int v)
{
    int r;
    switch (v)
    {
        case 1:
        case 2:
            r = 5;
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

### Violation (must be reported): a clause that ends in return (strict reading)

`main.c`

```c
int g(int v)
{
    switch (v)
    {
        case 1: /*@V*/
            return 1;
        default:
            break;
    }
    return 0;
}
```

### Violation (must be reported): a clause ending with a conditional break only

`main.c`

```c
void f(int v, int c)
{
    switch (v)
    {
        case 1:
            if (c) { break; } else { break; }
        default:
            break;
    }
}
```

### Violation (must be reported): the last clause without break

`main.c`

```c
void f(int v)
{
    switch (v)
    {
        case 1: v = 0; break;
        default: v = 1;
    }
}
```

### Compliant (must stay silent): stacked labels sharing a clause

`main.c`

```c
void f(int v)
{
    switch (v)
    {
        case 1:
        case 2:
            v = 0;
            break;
        default:
            break;
    }
}
```

### Violation (must be reported): a clause ending with goto

`main.c`

```c
void f(int v)
{
    switch (v)
    {
        case 1: goto out;
        default: break;
    }
out:
    v = 0;
}
```

### Compliant (must stay silent): a clause that is a block ending in break

`main.c`

```c
int f(int v)
{
    int r = 0;
    switch (v)
    {
        case 1:
        {
            r = 1;
            break;
        }
        default:
            break;
    }
    return r;
}
```

### Violation (must be reported): a clause that ends in continue inside a loop

`main.c`

```c
int f(int n)
{
    int i;
    int r = 0;
    for (i = 0; i < n; i++)
    {
        switch (i)
        {
            case 1: /*@V*/
                r++;
                continue;
            default:
                break;
        }
    }
    return r;
}
```

