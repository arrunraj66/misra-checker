# Rule 7.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Literals and constants

## What the detector reports

Reports an integer or floating literal whose suffix uses a lowercase l.

Detection contract: Report an integer or floating literal whose suffix uses lowercase l.

## Known limits

Only the suffix is examined.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | uppercase suffixes | pass |
| Violation (must be reported) | lowercase l on an integer | pass |
| Violation (must be reported) | lowercase l combined with u, and on long long | pass |
| Violation (must be reported) | lowercase l on a floating literal | pass |

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

