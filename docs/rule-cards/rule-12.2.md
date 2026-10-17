# Rule 12.2

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Expressions

## What the detector reports

Reports a constant shift count that is negative or not smaller than the width of the left operand.

Detection contract: Report a constant shift count that is negative or not smaller than the essential width of the left operand.

## Known limits

Only constant shift counts are examined.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | counts within the width | pass |
| Violation (must be reported) | count equal to the width | pass |
| Violation (must be reported) | negative count | pass |
| Violation (must be reported) | count too big for the type of the left operand | pass |

## Case sources

### Compliant (must stay silent): counts within the width

`main.c`

```c
unsigned int a = 1U << 31;
unsigned char b(unsigned char x) { return (unsigned char)(x << 3); }
```

### Violation (must be reported): count equal to the width

`main.c`

```c
unsigned int f(unsigned int x) { return x << 32; } /*@V*/
```

### Violation (must be reported): negative count

`main.c`

```c
int g(int x) { return x << -1; } /*@V*/
```

### Violation (must be reported): count too big for the type of the left operand

`main.c`

```c
long long h(long long x) { return x >> 64; } /*@V*/
```

