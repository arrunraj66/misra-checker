# Rule 7.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Literals and constants

## What the detector reports

Reports an integer literal of unsigned type that is written without a u or U suffix.

Detection contract: Report an integer literal of unsigned type that is spelled without a u or U suffix.

## Known limits

Only integer literals are examined. Whether a literal is unsigned depends on the target's integer sizes.

## Tested cases

13 of 13 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | decimal and hexadecimal literals that fit int | pass |
| Compliant (must stay silent) | unsigned literals with the suffix | pass |
| Compliant (must stay silent) | a decimal literal too big for int is a signed long, not unsigned | pass |
| Violation (must be reported) | hexadecimal literal that is unsigned int without a suffix | pass |
| Violation (must be reported) | octal-style literal that is unsigned without a suffix (also Rule 7.1) | pass |
| Violation (must be reported) | a decimal literal above LONG_MAX is unsigned long and needs the suffix | pass |
| Compliant (must stay silent) | a decimal literal beyond int is signed long in C99 | pass |
| Compliant (must stay silent) | negative constants and ones that fit | pass |
| Violation (must be reported) | hexadecimal unsigned literal used as an array index | pass |
| Compliant (must stay silent) | a literal inside a preprocessor condition is not an object | pass |
| Compliant (must stay silent) | suffix in lower and upper case combinations | pass |
| Compliant (must stay silent) | a signed hexadecimal literal that fits int | pass |
| Violation (must be reported) | a hexadecimal literal that does not fit a signed long | pass |

## Case sources

### Compliant (must stay silent): decimal and hexadecimal literals that fit int

`main.c`

```c
int a = 10;
int b = 0x7FFF;
```

### Compliant (must stay silent): unsigned literals with the suffix

`main.c`

```c
unsigned int a = 0xFFFFFFFFU;
unsigned long b = 10UL;
```

### Compliant (must stay silent): a decimal literal too big for int is a signed long, not unsigned

`main.c`

```c
long a = 4294967295;
```

### Violation (must be reported): hexadecimal literal that is unsigned int without a suffix

`main.c`

```c
unsigned int a = 0xFFFFFFFF; /*@V*/
```

### Violation (must be reported): octal-style literal that is unsigned without a suffix (also Rule 7.1)

`main.c`

```c
unsigned int b = 037777777777; /*@V*/
```

### Violation (must be reported): a decimal literal above LONG_MAX is unsigned long and needs the suffix

`main.c`

```c
unsigned long a = 9223372036854775808; /*@V*/
```

### Compliant (must stay silent): a decimal literal beyond int is signed long in C99

`main.c`

```c
long a = 4294967295;
```

### Compliant (must stay silent): negative constants and ones that fit

`main.c`

```c
int a = -1;
int b = 2147483647;
```

### Violation (must be reported): hexadecimal unsigned literal used as an array index

`main.c`

```c
int t[4];
int f(void) { return t[0x80000000 >> 31]; } /*@V*/
```

### Compliant (must stay silent): a literal inside a preprocessor condition is not an object

`main.c`

```c
#if 0xFFFFFFFF > 1
int a;
#endif
```

### Compliant (must stay silent): suffix in lower and upper case combinations

`main.c`

```c
unsigned int a = 0xFFFFFFFFu;
unsigned long b = 0xFFFFFFFFFFFFFFFFUL;
```

### Compliant (must stay silent): a signed hexadecimal literal that fits int

`main.c`

```c
int a = 0x7FFFFFFF;
```

### Violation (must be reported): a hexadecimal literal that does not fit a signed long

`main.c`

```c
unsigned long a = 0xFFFFFFFFFFFFFFFF; /*@V*/
```

