# Rule 21.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports #define or #undef of the name defined or of a reserved name (a name starting with underscore and a capital letter or a second underscore).

Detection contract: Report #define or #undef of defined or an underscore-reserved name.

## Known limits

Names reserved for the standard library by their prefix are not covered.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary macro names | pass |
| Violation (must be reported) | a name with a leading double underscore | pass |
| Violation (must be reported) | underscore and a capital letter | pass |
| Violation (must be reported) | #undef of a reserved name | pass |

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

