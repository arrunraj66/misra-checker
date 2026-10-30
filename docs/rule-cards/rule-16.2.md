# Rule 16.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Switch statements

## What the detector reports

Reports a case or default label that is not a top-level clause of its switch body.

Detection contract: Report a case or default label that is not a top-level clause of its switch body.

## Known limits

Only labels nested in blocks inside the switch body are reported.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | top-level labels | pass |
| Violation (must be reported) | a case label inside a nested block | pass |
| Violation (must be reported) | a label inside an if within the switch | pass |
| Violation (must be reported) | a case in a loop inside the switch (Duff's device) | pass |
| Compliant (must stay silent) | nested switch has its own top-level labels | pass |
| Violation (must be reported) | a case label inside an if within the switch | pass |
| Violation (must be reported) | a case label inside a nested block of the switch | pass |
| Compliant (must stay silent) | a case label inside a nested switch that owns it | pass |

## Case sources

### Compliant (must stay silent): top-level labels

`main.c`

```c
int f(int v) { switch (v) { case 1: return 1; default: return 0; } }
```

### Violation (must be reported): a case label inside a nested block

`main.c`

```c
int f(int v)
{
    int r = 0;
    switch (v)
    {
        case 1:
            r = 1;
            break;
        default:
            {
            case 2: /*@V*/
                r = 2;
            }
            break;
    }
    return r;
}
```

### Violation (must be reported): a label inside an if within the switch

`main.c`

```c
int g(int v)
{
    switch (v)
    {
        case 1:
            if (v > 0)
            {
        case 3: /*@V*/
                return 3;
            }
            return 1;
        default:
            return 0;
    }
}
```

### Violation (must be reported): a case in a loop inside the switch (Duff's device)

`main.c`

```c
void f(int v, int n)
{
    switch (v)
    {
        case 0:
            while (n--)
            {
        case 1: /*@V*/
                v++;
            }
            break;
        default:
            break;
    }
}
```

### Compliant (must stay silent): nested switch has its own top-level labels

`main.c`

```c
void f(int a, int b)
{
    switch (a)
    {
        case 1:
            switch (b) { case 1: b = 0; break; default: break; }
            break;
        default:
            break;
    }
}
```

### Violation (must be reported): a case label inside an if within the switch

`main.c`

```c
void f(int v, int c)
{
    switch (v)
    {
        case 1:
            if (c)
            {
        case 2: /*@V*/
                v = 0;
            }
            break;
        default:
            break;
    }
}
```

### Violation (must be reported): a case label inside a nested block of the switch

`main.c`

```c
void f(int x)
{
    switch (x) {
    case 1:
        {
        case 2: /*@V*/
            break;
        }
    default:
        break;
    }
}
```

### Compliant (must stay silent): a case label inside a nested switch that owns it

`main.c`

```c
void f(int x, int y)
{
    switch (x) {
    case 1:
        switch (y) {
        case 2:
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
```

