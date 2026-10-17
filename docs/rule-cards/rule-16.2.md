# Rule 16.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Switch statements

## What the detector reports

Reports a case or default label that is not a top-level clause of its switch body.

Detection contract: Report a case or default label that is not a top-level clause of its switch body.

## Known limits

Only labels nested in blocks inside the switch body are reported.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | top-level labels | pass |
| Violation (must be reported) | a case label inside a nested block | pass |
| Violation (must be reported) | a label inside an if within the switch | pass |

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

