# Rule 21.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports #define or #undef of the name defined or of a reserved name (a name starting with underscore and a capital letter or a second underscore).

Detection contract: Report #define or #undef of defined or an underscore-reserved name.

## Known limits

Names reserved for the standard library by their prefix are not covered.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary macro names | pass |
| Violation (must be reported) | a name with a leading double underscore | pass |
| Violation (must be reported) | underscore and a capital letter | pass |
| Violation (must be reported) | #undef of a reserved name | pass |
| Violation (must be reported) | defining a standard macro name | pass |
| Violation (must be reported) | defining a keyword-like reserved name | pass |
| Compliant (must stay silent) | a name with an underscore inside | pass |
| Violation (must be reported) | undefining a predefined macro | pass |

## Case sources

### Compliant (must stay silent): ordinary macro names

`main.c`

```c
#define LIMIT 10
#define my_macro(x) (x)
```

### Violation (must be reported): a name with a leading double underscore

`main.c`

```c
#define __INTERNAL 1 /*@V*/
```

### Violation (must be reported): underscore and a capital letter

`main.c`

```c
#define _Reserved 2 /*@V*/
```

### Violation (must be reported): #undef of a reserved name

`main.c`

```c
#undef __FILE__ /*@V*/
```

### Violation (must be reported): defining a standard macro name

`main.c`

```c
#define errno 0 /*@V*/
int x;
```

### Violation (must be reported): defining a keyword-like reserved name

`main.c`

```c
#define _Reserved 1 /*@V*/
int x;
```

### Compliant (must stay silent): a name with an underscore inside

`main.c`

```c
#define MY_LEVEL 1
int x;
```

### Violation (must be reported): undefining a predefined macro

`main.c`

```c
#undef __STDC__ /*@V*/
int x;
```

