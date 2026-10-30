# Rule 10.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Reports a composite expression used as an operand together with an operand of a wider non-constant essential type.

Detection contract: Report a composite expression used as operand with a wider non-constant operand.

## Known limits

Constants are exempt.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | both operands of the same width | pass |
| Compliant (must stay silent) | a constant operand | pass |
| Violation (must be reported) | a composite short operand combined with an int | pass |
| Violation (must be reported) | a composite char-width operand combined with a long | pass |
| Violation (must be reported) | composite int combined with long | pass |
| Compliant (must stay silent) | composite widened first | pass |
| Violation (must be reported) | a composite float combined with a double | pass |

## Case sources

### Compliant (must stay silent): both operands of the same width

`main.c`

```c
int f(short a, short b, short c) { return (a + b) * c; }
```

### Compliant (must stay silent): a constant operand

`main.c`

```c
int g(short a, short b) { return (a + b) * 2; }
```

### Violation (must be reported): a composite short operand combined with an int

`main.c`

```c
int f(short a, short b, int c) { return (a + b) * c; } /*@V*/
```

### Violation (must be reported): a composite char-width operand combined with a long

`main.c`

```c
long g(unsigned char a, unsigned char b, long c) { return (a + b) + c; } /*@V*/
```

### Violation (must be reported): composite int combined with long

`main.c`

```c
long f(int a, int b, long c) { return (a + b) + c; } /*@V*/
```

### Compliant (must stay silent): composite widened first

`main.c`

```c
long f(int a, int b, long c) { return ((long)a + b) + c; }
```

### Violation (must be reported): a composite float combined with a double

`main.c`

```c
double f(float a, float b, double c) { return (a * b) + c; } /*@V*/
```

