# Rule 20.12

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a macro parameter that is used both as an operand of # or ## and elsewhere in the replacement list.

Detection contract: Report a macro parameter used both as an operand of # or ## and elsewhere.

## Known limits

Whether the second use is expanded is not analysed; the combined use is reported.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | used only with the operator | pass |
| Compliant (must stay silent) | used only without the operator | pass |
| Violation (must be reported) | used with # and also on its own | pass |
| Violation (must be reported) | used with ## and also on its own | pass |

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

