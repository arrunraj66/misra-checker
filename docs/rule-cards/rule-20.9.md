# Rule 20.9

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports an identifier in an #if or #elif condition that is not a defined macro at that point (operands of defined are fine).

Detection contract: Report an identifier in an #if/#elif condition that is not a defined macro at that point.

## Known limits

Integer suffixes such as 1U are numbers, not identifiers.

## Tested cases

11 of 11 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | defined macros and defined() | pass |
| Compliant (must stay silent) | suffixed numbers | pass |
| Violation (must be reported) | an identifier that is not defined | pass |
| Violation (must be reported) | undefined in an elif | pass |
| Violation (must be reported) | true is not defined in C99 | pass |
| Compliant (must stay silent) | identifier guarded by defined in the same condition | pass |
| Compliant (must stay silent) | function-like macro defined and called | pass |
| Violation (must be reported) | an undefined name on the right of a comparison | pass |
| Compliant (must stay silent) | macros defined on the command line style via a prior define | pass |
| Compliant (must stay silent) | the __STDC_VERSION__ predefined macro | pass |
| Compliant (must stay silent) | __has_include style checks are not identifiers to define | pass |

## Case sources

### Compliant (must stay silent): defined macros and defined()

`main.c`

```c
#define LEVEL 2
#if LEVEL > 1
int a;
#endif
#if defined(UNDEFINED_OK)
int b;
#endif
#ifdef ALSO_FINE
int c;
#endif
```

### Compliant (must stay silent): suffixed numbers

`main.c`

```c
#if 4U > 1UL
int d;
#endif
```

### Violation (must be reported): an identifier that is not defined

`main.c`

```c
#if UNDEFINED_SYMBOL > 0 /*@V*/
int a;
#endif
```

### Violation (must be reported): undefined in an elif

`main.c`

```c
#define A 1
#if A
int a;
#elif B_UNDEFINED /*@V*/
int b;
#endif
```

### Violation (must be reported): true is not defined in C99

`main.c`

```c
#if true /*@V*/
int a;
#endif
```

### Compliant (must stay silent): identifier guarded by defined in the same condition

`main.c`

```c
#define X 2
#if defined(X) && X > 1
int a;
#endif
```

### Compliant (must stay silent): function-like macro defined and called

`main.c`

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#if MAX(1, 2) > 1
int a;
#endif
```

### Violation (must be reported): an undefined name on the right of a comparison

`main.c`

```c
#define A 1
#if A == B /*@V*/
int a;
#endif
```

### Compliant (must stay silent): macros defined on the command line style via a prior define

`main.c`

```c
#define CONFIG_LEVEL 3
#if CONFIG_LEVEL >= 2 && defined(CONFIG_LEVEL)
int a;
#endif
```

### Compliant (must stay silent): the __STDC_VERSION__ predefined macro

`main.c`

```c
#if __STDC_VERSION__ >= 199901L
int a;
#endif
```

### Compliant (must stay silent): __has_include style checks are not identifiers to define

`main.c`

```c
#ifdef __has_include
int b;
#endif
```

