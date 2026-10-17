# Rule 10.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Reports a character operand used inappropriately in an addition or subtraction.

Detection contract: Report character operands used inappropriately in addition or subtraction.

## Known limits

Only addition and subtraction are examined.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a digit converted to a number and back | pass |
| Compliant (must stay silent) | character compared and not computed | pass |
| Violation (must be reported) | two characters added | pass |
| Violation (must be reported) | a character added to a character constant | pass |

## Case sources

### Compliant (must stay silent): a digit converted to a number and back

`main.c`

```c
int to_int(char c) { return c - '0'; }
char to_char(int d) { return (char)('0' + d); }
```

### Compliant (must stay silent): character compared and not computed

`main.c`

```c
int is_a(char c) { return c == 'a'; }
```

### Violation (must be reported): two characters added

`main.c`

```c
int f(char a, char b) { return a + b; } /*@V*/
```

### Violation (must be reported): a character added to a character constant

`main.c`

```c
int g(char a) { return a + 'b'; } /*@V*/
```

