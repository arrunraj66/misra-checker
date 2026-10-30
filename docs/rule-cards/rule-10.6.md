# Rule 10.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Reports a composite arithmetic expression whose value is assigned to an object of a wider essential type.

Detection contract: Report a composite arithmetic expression assigned to an object of wider essential type.

## Known limits

The essential type of the expression is that of its operands (no integer promotion).

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | the operands are widened before the operation | pass |
| Compliant (must stay silent) | assignment to an object of the same width | pass |
| Violation (must be reported) | a short sum assigned to an int | pass |
| Violation (must be reported) | an unsigned char product assigned to an unsigned int | pass |
| Compliant (must stay silent) | composite computed in the wider type | pass |
| Violation (must be reported) | a signed char sum stored in a long | pass |
| Compliant (must stay silent) | a non-composite operand assigned to a wider type | pass |

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

### Compliant (must stay silent): composite computed in the wider type

`main.c`

```c
unsigned int f(unsigned short a, unsigned short b) { unsigned int r = (unsigned int)a + b; return r; }
```

### Violation (must be reported): a signed char sum stored in a long

`main.c`

```c
long f(signed char a, signed char b) { long r = a + b; return r; } /*@V*/
```

### Compliant (must stay silent): a non-composite operand assigned to a wider type

`main.c`

```c
long f(int a) { long r = a; return r; }
```

