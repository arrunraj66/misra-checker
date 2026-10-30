# Rule 20.12

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a macro parameter that is used both as an operand of # or ## and elsewhere in the replacement list.

Detection contract: Report a macro parameter used both as an operand of # or ## and elsewhere.

## Known limits

Whether the second use is expanded is not analysed; the combined use is reported.

- **False negative:** a macro name passed as the argument of a stringizing macro (only the macro definition is examined) (documented limit)

## Tested cases

9 of 10 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | used only with the operator | pass |
| Compliant (must stay silent) | used only without the operator | pass |
| Violation (must be reported) | used with # and also on its own | pass |
| Violation (must be reported) | used with ## and also on its own | pass |
| Compliant (must stay silent) | the two-level stringizing idiom | pass |
| Violation (must be reported) | pasting an operand that is also used on its own | pass |
| Known false negative (not detected yet) | a macro name passed as the argument of a stringizing macro (only the macro definition is examined) | documented limit |
| Compliant (must stay silent) | a parameter only stringized | pass |
| Compliant (must stay silent) | a parameter used twice and never with # or ## | pass |
| Violation (must be reported) | pasting and plain use in one replacement list | pass |

## Case sources

### Compliant (must stay silent): used only with the operator

`main.c`

```c
#define S(x) #x
const char *s = S(abc);
```

### Compliant (must stay silent): used only without the operator

`main.c`

```c
#define ID(x) (x)
int v = ID(1);
```

### Violation (must be reported): used with # and also on its own

`main.c`

```c
#define SHOW(x) puts(#x), x /*@V*/
int puts(const char *);
int v = (SHOW(1), 0);
```

### Violation (must be reported): used with ## and also on its own

`main.c`

```c
#define TWICE(x) x##_a + x /*@V*/
int q_a = 1, q = 2;
int v = TWICE(q);
```

### Compliant (must stay silent): the two-level stringizing idiom

`main.c`

```c
#define STR(x) #x
#define XSTR(x) STR(x)
const char *s = XSTR(42);
```

### Violation (must be reported): pasting an operand that is also used on its own

`main.c`

```c
#define MK(x) x##_t x /*@V*/
typedef int MK(a);
```

### Known false negative (not detected yet): a macro name passed as the argument of a stringizing macro (only the macro definition is examined)

`main.c`

```c
#define A 1
#define S(x) #x
const char *s = S(A); /*@V*/
```

### Compliant (must stay silent): a parameter only stringized

`main.c`

```c
#define S(x) #x
const char *s = S(abc);
```

### Compliant (must stay silent): a parameter used twice and never with # or ##

`main.c`

```c
#define TWICE(x) ((x) + (x))
int v = TWICE(2);
```

### Violation (must be reported): pasting and plain use in one replacement list

`main.c`

```c
#define MIX(a, b) a##b + a /*@V*/
int xy = 1, x = 2;
int v = MIX(x, y);
```

