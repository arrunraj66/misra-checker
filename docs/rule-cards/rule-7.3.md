# Rule 7.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Literals and constants

## What the detector reports

Reports an integer or floating literal whose suffix uses a lowercase l.

Detection contract: Report an integer or floating literal whose suffix uses lowercase l.

## Known limits

Only the suffix is examined.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | uppercase suffixes | pass |
| Violation (must be reported) | lowercase l on an integer | pass |
| Violation (must be reported) | lowercase l combined with u, and on long long | pass |
| Violation (must be reported) | lowercase l on a floating literal | pass |
| Violation (must be reported) | lowercase l on a hexadecimal literal | pass |
| Compliant (must stay silent) | lowercase letters that are not a suffix l | pass |
| Compliant (must stay silent) | uppercase suffix combinations | pass |
| Compliant (must stay silent) | l inside a string and character constant | pass |
| Violation (must be reported) | lowercase ll | pass |

## Case sources

### Compliant (must stay silent): uppercase suffixes

`main.c`

```c
long a = 10L;
unsigned long b = 10UL;
long long c = 10LL;
long double d = 1.0L;
float e = 1.0f;
```

### Violation (must be reported): lowercase l on an integer

`main.c`

```c
long a = 10l; /*@V*/
```

### Violation (must be reported): lowercase l combined with u, and on long long

`main.c`

```c
unsigned long b = 10ul; /*@V*/
long long c = 10ll; /*@V*/
```

### Violation (must be reported): lowercase l on a floating literal

`main.c`

```c
long double d = 1.0l; /*@V*/
```

### Violation (must be reported): lowercase l on a hexadecimal literal

`main.c`

```c
long a = 0x1l; /*@V*/
```

### Compliant (must stay silent): lowercase letters that are not a suffix l

`main.c`

```c
int a = 0xabc;
float f = 1.5f;
double d = 1e3;
```

### Compliant (must stay silent): uppercase suffix combinations

`main.c`

```c
unsigned long long a = 1ULL;
long double b = 1.0L;
```

### Compliant (must stay silent): l inside a string and character constant

`main.c`

```c
const char *s = "1l";
int c = 'l';
```

### Violation (must be reported): lowercase ll

`main.c`

```c
long long a = 1ll; /*@V*/
```

