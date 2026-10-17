# Rule 8.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports a declaration written without a type specifier (implicit int).

Detection contract: Report a declaration with no written type specifier (implicit int).

## Known limits

Only declarations that the compiler accepts with a warning are reachable; strict compilers reject them as errors.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | explicit types | pass |
| Violation (must be reported) | object declared without a type | pass |
| Violation (must be reported) | function defined without a return type | pass |

## Case sources

### Compliant (must stay silent): explicit types

`main.c`

```c
static int a = 3;
int f(void) { return 0; }
```

### Violation (must be reported): object declared without a type

`main.c`

```c
static a = 3; /*@V*/
```

### Violation (must be reported): function defined without a return type

`main.c`

```c
f(void) { return 0; } /*@V*/
```

