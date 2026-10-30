# Rule 16.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Switch statements

## What the detector reports

Reports a switch whose controlling expression is essentially Boolean.

Detection contract: Report a switch whose controlling expression is essentially Boolean.

## Known limits

Only the type of the controlling expression is examined.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | an integer controlling expression | pass |
| Violation (must be reported) | a comparison as the controlling expression | pass |
| Violation (must be reported) | a _Bool controlling expression | pass |
| Violation (must be reported) | negation as the controlling expression | pass |
| Compliant (must stay silent) | character controlling expression | pass |
| Compliant (must stay silent) | enum controlling expression | pass |
| Violation (must be reported) | a switch on a comparison | pass |
| Compliant (must stay silent) | a switch on an integer | pass |

## Case sources

### Compliant (must stay silent): an integer controlling expression

`main.c`

```c
int f(int v) { switch (v) { case 1: return 1; default: return 0; } }
```

### Violation (must be reported): a comparison as the controlling expression

`main.c`

```c
int f(int a, int b)
{
    switch (a == b) /*@V*/
    {
        case 0: return 0;
        default: return 1;
    }
}
```

### Violation (must be reported): a _Bool controlling expression

`main.c`

```c
int g(_Bool flag)
{
    switch (flag) /*@V*/
    {
        case 0: return 0;
        default: return 1;
    }
}
```

### Violation (must be reported): negation as the controlling expression

`main.c`

```c
void f(int v)
{
    switch (!v) /*@V*/
    {
        case 0: break;
        default: break;
    }
}
```

### Compliant (must stay silent): character controlling expression

`main.c`

```c
void f(char c)
{
    switch (c)
    {
        case 'a': break;
        default: break;
    }
}
```

### Compliant (must stay silent): enum controlling expression

`main.c`

```c
enum e { A, B };
void f(enum e v)
{
    switch (v)
    {
        case A: break;
        default: break;
    }
}
```

### Violation (must be reported): a switch on a comparison

`main.c`

```c
void f(int x)
{
    switch (x == 1) { /*@V*/
    case 0:
        break;
    default:
        break;
    }
}
```

### Compliant (must stay silent): a switch on an integer

`main.c`

```c
void f(int x)
{
    switch (x) {
    case 0:
        break;
    default:
        break;
    }
}
```

