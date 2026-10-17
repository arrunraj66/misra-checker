# Rule 10.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Reports a composite arithmetic expression whose value is assigned to an object of a wider essential type.

Detection contract: Report a composite arithmetic expression assigned to an object of wider essential type.

## Known limits

The essential type of the expression is that of its operands (no integer promotion).

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | the operands are widened before the operation | pass |
| Compliant (must stay silent) | assignment to an object of the same width | pass |
| Violation (must be reported) | a short sum assigned to an int | pass |
| Violation (must be reported) | an unsigned char product assigned to an unsigned int | pass |

## Case sources

### Compliant (must stay silent): the operands are widened before the operation

`main.c`

```c
int f(short a, short b) { int r = (int)a + (int)b; return r; }
```

### Compliant (must stay silent): assignment to an object of the same width

`main.c`

```c
short g(short a, short b) { short r = (short)(a + b); return r; }
```

### Violation (must be reported): a short sum assigned to an int

`main.c`

```c
int f(short a, short b)
{
    int r = a + b; /*@V*/
    return r;
}
```

### Violation (must be reported): an unsigned char product assigned to an unsigned int

`main.c`

```c
unsigned int g(unsigned char a, unsigned char b)
{
    unsigned int r;
    r = a * b; /*@V*/
    return r;
}
```

