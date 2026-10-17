# Rule 10.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Reports arithmetic, relational, equality and bitwise operators whose non-constant operands have different essential type categories.

Detection contract: Report binary arithmetic, relational, equality and bitwise operators whose non-constant operands have different essential type categories.

## Known limits

Constants are exempt. The essential type of arithmetic on small integers is computed without promotion.

## Tested cases

10 of 10 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | same category | pass |
| Compliant (must stay silent) | a constant operand of another category | pass |
| Compliant (must stay silent) | small unsigned arithmetic is unsigned | pass |
| Violation (must be reported) | signed with unsigned | pass |
| Violation (must be reported) | integer with floating | pass |
| Violation (must be reported) | Boolean with integer in a comparison | pass |
| Compliant (must stay silent) | comparisons of the same category | pass |
| Compliant (must stay silent) | operands that are constants | pass |
| Violation (must be reported) | two different enumerations compared | pass |
| Violation (must be reported) | unsigned with signed variables | pass |

## Case sources

### Compliant (must stay silent): same category

`main.c`

```c
int f(int a, int b) { return a + b; }
int g(unsigned int a, unsigned int b) { return a < b; }
```

### Compliant (must stay silent): a constant operand of another category

`main.c`

```c
int h(unsigned int u) { return u + 1; }
```

### Compliant (must stay silent): small unsigned arithmetic is unsigned

`main.c`

```c
int k(unsigned char a, unsigned char b, unsigned char c) { return a == (b - c); }
```

### Violation (must be reported): signed with unsigned

`main.c`

```c
int f(int a, unsigned int b) { return a + b; } /*@V*/
```

### Violation (must be reported): integer with floating

`main.c`

```c
int g(int a, double b) { return a < b; } /*@V*/
```

### Violation (must be reported): Boolean with integer in a comparison

`main.c`

```c
int h(_Bool f, int a) { return f == a; } /*@V*/
```

### Compliant (must stay silent): comparisons of the same category

`main.c`

```c
int f(char a, char b) { return a == b; }
int g(unsigned char a, unsigned int b) { return a < b; }
```

### Compliant (must stay silent): operands that are constants

`main.c`

```c
int h(unsigned int u) { return u > 10; }
```

### Violation (must be reported): two different enumerations compared

`main.c`

```c
enum color { RED, GREEN };
enum shape { ROUND, SQUARE };
int f(enum color c, enum shape s) { return c == s; } /*@V*/
```

### Violation (must be reported): unsigned with signed variables

`main.c`

```c
int g(unsigned int u, int i) { return u < i; } /*@V*/
```

