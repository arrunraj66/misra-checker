# Rule 20.11

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a macro in which a # operand is immediately followed by ##.

Detection contract: Report a macro in which a # operand is immediately followed by ##.

## Known limits

The # then ## sequence is reported at the macro definition.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | # alone and ## alone | pass |
| Violation (must be reported) | stringizing then pasting | pass |
| Compliant (must stay silent) | a pasting of two parameters and a stringizing of one | pass |
| Violation (must be reported) | stringize then paste in an object-like context with two parameters | pass |
| Compliant (must stay silent) | paste then stringize in separate macros | pass |
| Compliant (must stay silent) | a stringized parameter next to a paste of other operands | pass |
| Compliant (must stay silent) | hash inside a string literal followed by a paste | pass |
| Compliant (must stay silent) | variadic stringizing | pass |

## Case sources

### Compliant (must stay silent): # alone and ## alone

`main.c`

```c
#define S(x) #x
#define C(a, b) a##b
const char *s = S(q);
int C(v, 1);
```

### Violation (must be reported): stringizing then pasting

`main.c`

```c
#define BAD(x) #x ## _suffix /*@V*/
const char *s = BAD(q);
```

### Compliant (must stay silent): a pasting of two parameters and a stringizing of one

`main.c`

```c
#define CAT(a, b) a##b
#define STR(a) #a
int CAT(v, 1);
const char *s = STR(q);
```

### Violation (must be reported): stringize then paste in an object-like context with two parameters

`main.c`

```c
#define M(a, b) #a ## b /*@V*/
const char *s = M(x, y);
```

### Compliant (must stay silent): paste then stringize in separate macros

`main.c`

```c
#define P(a, b) a ## b
#define S(a) #a
#define SX(a) S(a)
const char *s = SX(P(1, 2));
```

### Compliant (must stay silent): a stringized parameter next to a paste of other operands

`main.c`

```c
#define M(a, b, c) #a, b ## c
int v12 = 3;
const char *s[] = {M(x, v, 12)};
```

### Compliant (must stay silent): hash inside a string literal followed by a paste

`main.c`

```c
#define M(a) "#" a
const char *s = M("x");
```

### Compliant (must stay silent): variadic stringizing

`main.c`

```c
#define LOG(...) #__VA_ARGS__
const char *s = LOG(a, b);
```

