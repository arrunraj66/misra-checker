# Rule 10.8

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Reports a cast of a composite expression to a wider or different essential type.

Detection contract: Report a cast of a composite expression to a wider or different essential type.

## Known limits

Only explicit casts are examined.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | cast to the same width | pass |
| Compliant (must stay silent) | cast of a non-composite operand to a wider type | pass |
| Violation (must be reported) | composite short sum cast to int | pass |
| Violation (must be reported) | composite unsigned char product cast to unsigned long | pass |

## Case sources

### Compliant (must stay silent): cast to the same width

`main.c`

```c
short f(short a, short b) { return (short)(a + b); }
```

### Compliant (must stay silent): cast of a non-composite operand to a wider type

`main.c`

```c
int g(short a) { return (int)a; }
```

### Violation (must be reported): composite short sum cast to int

`main.c`

```c
int f(short a, short b) { return (int)(a + b); } /*@V*/
```

### Violation (must be reported): composite unsigned char product cast to unsigned long

`main.c`

```c
unsigned long g(unsigned char a, unsigned char b) { return (unsigned long)(a * b); } /*@V*/
```

