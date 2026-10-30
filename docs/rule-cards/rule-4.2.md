# Rule 4.2

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Character sets and lexical elements

## What the detector reports

Reports a trigraph sequence found in the raw text of the main file.

Detection contract: Report a trigraph sequence found in the raw text of the main file.

## Known limits

Detected on the raw text, also inside comments and strings. Compilers with trigraphs disabled still contain the sequence, so it is reported.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | question marks that do not form a trigraph | pass |
| Violation (must be reported) | trigraph in a string | pass |
| Violation (must be reported) | trigraph in code position | pass |
| Violation (must be reported) | question-mark trigraph for a bracket | pass |
| Violation (must be reported) | trigraph in a character constant | pass |
| Compliant (must stay silent) | a lone question mark pair at the end of a string | pass |
| Compliant (must stay silent) | a ternary expression with question marks apart | pass |

## Case sources

### Compliant (must stay silent): question marks that do not form a trigraph

`main.c`

```c
const char *a = "what? really?";
const char *b = "??";
```

### Violation (must be reported): trigraph in a string

`main.c`

```c
const char *a = "??="; /*@V*/
```

### Violation (must be reported): trigraph in code position

`main.c`

```c
int a = 0; ??/ /*@V*/
```

### Violation (must be reported): question-mark trigraph for a bracket

`main.c`

```c
char *s = "??)"; /*@V*/
```

### Violation (must be reported): trigraph in a character constant

`main.c`

```c
int c = '??/'; /*@V*/
```

### Compliant (must stay silent): a lone question mark pair at the end of a string

`main.c`

```c
const char *s = "what??";
```

### Compliant (must stay silent): a ternary expression with question marks apart

`main.c`

```c
int f(int a) { return a ? a ? 1 : 2 : 3; }
```

