# Rule 12.1

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Expressions

## What the detector reports

Strict reading: reports a binary operator that has an unparenthesized binary operand of a different precedence level.

Detection contract: Strict reading: report a binary operator with an unparenthesized binary operand of a different precedence level.

## Known limits

Chains of the same precedence level are accepted; this is the strict interpretation of the rule.

## Tested cases

6 of 6 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | explicit parentheses | pass |
| Compliant (must stay silent) | operators of the same level | pass |
| Compliant (must stay silent) | parenthesized logical mix | pass |
| Violation (must be reported) | addition mixed with multiplication | pass |
| Violation (must be reported) | logical and mixed with logical or | pass |
| Violation (must be reported) | shift mixed with addition | pass |

## Case sources

### Compliant (must stay silent): explicit parentheses

`main.c`

```c
int f(int a, int b, int c) { return a + (b * c); }
```

### Compliant (must stay silent): operators of the same level

`main.c`

```c
int g(int a, int b, int c) { return a + b + c; }
int h(int a, int b, int c) { return a * b * c; }
```

### Compliant (must stay silent): parenthesized logical mix

`main.c`

```c
int k(int a, int b, int c) { return (a && b) || c; }
```

### Violation (must be reported): addition mixed with multiplication

`main.c`

```c
int f(int a, int b, int c) { return a + b * c; } /*@V*/
```

### Violation (must be reported): logical and mixed with logical or

`main.c`

```c
int g(int a, int b, int c) { return a || b && c; } /*@V*/
```

### Violation (must be reported): shift mixed with addition

`main.c`

```c
int h(int a, int b) { return a << b + 1; } /*@V*/
```

