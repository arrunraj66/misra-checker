# Rule 7.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Literals and constants

## What the detector reports

Reports an integer literal that is written with a leading zero followed by further digits (an octal constant), including one in an unexpanded macro body.

Detection contract: Report an integer literal spelled with a leading zero followed by further digits.

## Known limits

A plain 0 is not octal. Octal escape sequences in strings are Rule 4.1 and 7.x does not examine them.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | zero, decimal, hexadecimal and floating literals | pass |
| Compliant (must stay silent) | an octal escape inside a string is not an octal constant | pass |
| Violation (must be reported) | octal constant | pass |
| Violation (must be reported) | octal constant in an expression and as an array size | pass |
| Violation (must be reported) | zero followed by a zero digit | pass |
| Violation (must be reported) | octal constant in a macro body that is never expanded | pass |
| Compliant (must stay silent) | character literals and floating literals with leading zeros | pass |
| Compliant (must stay silent) | hexadecimal with a leading zero | pass |
| Violation (must be reported) | octal in a case label | pass |

## Case sources

### Compliant (must stay silent): zero, decimal, hexadecimal and floating literals

`main.c`

```c
int a = 0;
int b = 10;
int c = 0x1F;
double d = 0.5;
double e = 01.5e0;
unsigned int f = 0U;
```

### Compliant (must stay silent): an octal escape inside a string is not an octal constant

`main.c`

```c
const char *s = "\101";
```

### Violation (must be reported): octal constant

`main.c`

```c
int a = 010; /*@V*/
```

### Violation (must be reported): octal constant in an expression and as an array size

`main.c`

```c
int arr[0777]; /*@V*/
int f(int x) { return x + 011; } /*@V*/
```

### Violation (must be reported): zero followed by a zero digit

`main.c`

```c
int a = 00; /*@V*/
```

### Violation (must be reported): octal constant in a macro body that is never expanded

`main.c`

```c
#define MASK 0755 /*@V*/
```

### Compliant (must stay silent): character literals and floating literals with leading zeros

`main.c`

```c
char c = '\0';
double d = 00.5;
double e = 0e2;
```

### Compliant (must stay silent): hexadecimal with a leading zero

`main.c`

```c
int h = 0x0F;
```

### Violation (must be reported): octal in a case label

`main.c`

```c
int f(int v)
{
    switch (v)
    {
        case 010: return 1; /*@V*/
        default: return 0;
    }
}
```

