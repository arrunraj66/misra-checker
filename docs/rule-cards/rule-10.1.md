# Rule 10.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Subset: reports Boolean or enumerated operands of arithmetic operators, unary minus on unsigned operands, and operands of bitwise and shift operators that are not unsigned or not constants.

Detection contract: Subset: report Boolean or enumerated operands of arithmetic operators, unsigned unary minus, and non-unsigned or non-constant operands of bitwise and shift operators.

## Known limits

Only the listed operator and operand combinations are examined, not the whole operand table.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | unsigned operands of bitwise operators | pass |
| Compliant (must stay silent) | arithmetic on integers | pass |
| Violation (must be reported) | Boolean operand of an arithmetic operator | pass |
| Violation (must be reported) | enumerated operand of an arithmetic operator | pass |
| Violation (must be reported) | unary minus on an unsigned operand | pass |
| Violation (must be reported) | bitwise operator on signed non-constant operands | pass |
| Violation (must be reported) | shift of a signed non-constant value | pass |

## Case sources

### Compliant (must stay silent): unsigned operands of bitwise operators

`main.c`

```c
unsigned int f(unsigned int a, unsigned int b) { return (a & b) | (a << 1); }
```

### Compliant (must stay silent): arithmetic on integers

`main.c`

```c
int g(int a, int b) { return a + b * 2; }
```

### Violation (must be reported): Boolean operand of an arithmetic operator

`main.c`

```c
int f(_Bool flag) { return flag + 1; } /*@V*/
```

### Violation (must be reported): enumerated operand of an arithmetic operator

`main.c`

```c
enum e { A, B };
int g(enum e v) { return v + 1; } /*@V*/
```

### Violation (must be reported): unary minus on an unsigned operand

`main.c`

```c
int h(unsigned int u) { return (int)-u; } /*@V*/
```

### Violation (must be reported): bitwise operator on signed non-constant operands

`main.c`

```c
int k(int a, int b) { return a & b; } /*@V*/
```

### Violation (must be reported): shift of a signed non-constant value

`main.c`

```c
int m(int a) { return a << 2; } /*@V*/
```

