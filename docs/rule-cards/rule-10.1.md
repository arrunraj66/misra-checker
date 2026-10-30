# Rule 10.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Subset: reports Boolean or enumerated operands of arithmetic operators, unary minus on unsigned operands, and operands of bitwise and shift operators that are not unsigned or not constants.

Detection contract: Subset: report Boolean or enumerated operands of arithmetic operators, unsigned unary minus, and non-unsigned or non-constant operands of bitwise and shift operators.

## Known limits

Only the listed operator and operand combinations are examined, not the whole operand table.

## Tested cases

15 of 15 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | unsigned operands of bitwise operators | pass |
| Compliant (must stay silent) | arithmetic on integers | pass |
| Violation (must be reported) | Boolean operand of an arithmetic operator | pass |
| Violation (must be reported) | enumerated operand of an arithmetic operator | pass |
| Violation (must be reported) | unary minus on an unsigned operand | pass |
| Violation (must be reported) | bitwise operator on signed non-constant operands | pass |
| Violation (must be reported) | shift of a signed non-constant value | pass |
| Violation (must be reported) | logical negation of an integer | pass |
| Violation (must be reported) | logical and of integer operands | pass |
| Compliant (must stay silent) | logical operators on comparison results | pass |
| Violation (must be reported) | bitwise complement of a signed value | pass |
| Compliant (must stay silent) | bitwise complement of an unsigned value | pass |
| Compliant (must stay silent) | shift of an unsigned value by a signed constant count | pass |
| Violation (must be reported) | Boolean used as an array index in arithmetic | pass |
| Compliant (must stay silent) | Boolean combined logically | pass |

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

### Violation (must be reported): logical negation of an integer

`main.c`

```c
int f(int x) { return !x; } /*@V*/
```

### Violation (must be reported): logical and of integer operands

`main.c`

```c
int f(int x, int y) { return x && y; } /*@V*/
```

### Compliant (must stay silent): logical operators on comparison results

`main.c`

```c
int f(int x, int y) { return (x > 1) && (y < 2); }
```

### Violation (must be reported): bitwise complement of a signed value

`main.c`

```c
int f(int x) { return ~x; } /*@V*/
```

### Compliant (must stay silent): bitwise complement of an unsigned value

`main.c`

```c
unsigned int f(unsigned int x) { return ~x; }
```

### Compliant (must stay silent): shift of an unsigned value by a signed constant count

`main.c`

```c
unsigned int f(unsigned int x) { return x << 2; }
```

### Violation (must be reported): Boolean used as an array index in arithmetic

`main.c`

```c
int f(_Bool b, int x) { return x + b; } /*@V*/
```

### Compliant (must stay silent): Boolean combined logically

`main.c`

```c
int f(_Bool a, _Bool b) { return a && b; }
```

