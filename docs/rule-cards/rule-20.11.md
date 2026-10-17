# Rule 20.11

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a macro in which a # operand is immediately followed by ##.

Detection contract: Report a macro in which a # operand is immediately followed by ##.

## Known limits

The # then ## sequence is reported at the macro definition.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | # alone and ## alone | pass |
| Violation (must be reported) | stringizing then pasting | pass |

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

