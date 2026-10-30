# Rule 20.10

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a macro definition that uses the # or ## operator.

Detection contract: Report a macro definition that uses the # or ## operator.

## Known limits

Every use of either operator in a macro definition is reported.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | macros without the operators | pass |
| Violation (must be reported) | stringizing | pass |
| Violation (must be reported) | token pasting | pass |
| Violation (must be reported) | pasting with a prefix | pass |
| Compliant (must stay silent) | a macro that only uses its parameters | pass |
| Compliant (must stay silent) | a macro whose body mentions a hash inside a string literal | pass |
| Violation (must be reported) | both operators in different macros | pass |

## Case sources

### Compliant (must stay silent): macros without the operators

`main.c`

```c
#define LIMIT 10
#define ADD(a, b) ((a) + (b))
int x = ADD(LIMIT, 1);
```

### Violation (must be reported): stringizing

`main.c`

```c
#define STR(x) #x /*@V*/
const char *s = STR(abc);
```

### Violation (must be reported): token pasting

`main.c`

```c
#define CAT(a, b) a##b /*@V*/
int CAT(var, 1);
```

### Violation (must be reported): pasting with a prefix

`main.c`

```c
#define NAME(x) pre_##x /*@V*/
int NAME(a);
```

### Compliant (must stay silent): a macro that only uses its parameters

`main.c`

```c
#define TWICE(x) ((x) + (x))
int a = TWICE(1);
```

### Compliant (must stay silent): a macro whose body mentions a hash inside a string literal

`main.c`

```c
#define MSG "# not an operator"
const char *s = MSG;
```

### Violation (must be reported): both operators in different macros

`main.c`

```c
#define STR(x) #x /*@V*/
#define CAT(a, b) a##b /*@V*/
int CAT(v, 1);
const char *s = STR(a);
```

