# Rule 10.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Reports a character operand used inappropriately in an addition or subtraction.

Detection contract: Report character operands used inappropriately in addition or subtraction.

## Known limits

Only addition and subtraction are examined.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a digit converted to a number and back | pass |
| Compliant (must stay silent) | character compared and not computed | pass |
| Violation (must be reported) | two characters added | pass |
| Violation (must be reported) | a character added to a character constant | pass |
| Violation (must be reported) | integer minus character | pass |
| Compliant (must stay silent) | character minus character | pass |
| Compliant (must stay silent) | character plus an integer | pass |
| Compliant (must stay silent) | digit conversion | pass |

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

### Violation (must be reported): integer minus character

`main.c`

```c
int f(int i, char c) { return i - c; } /*@V*/
```

### Compliant (must stay silent): character minus character

`main.c`

```c
int f(char a, char b) { return a - b; }
```

### Compliant (must stay silent): character plus an integer

`main.c`

```c
char f(char c, int n) { return (char)(c + n); }
```

### Compliant (must stay silent): digit conversion

`main.c`

```c
int f(char c) { return c - '0'; }
```

