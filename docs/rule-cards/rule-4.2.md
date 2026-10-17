# Rule 4.2

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Character sets and lexical elements

## What the detector reports

Reports a trigraph sequence found in the raw text of the main file.

Detection contract: Report a trigraph sequence found in the raw text of the main file.

## Known limits

Detected on the raw text, also inside comments and strings. Compilers with trigraphs disabled still contain the sequence, so it is reported.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | question marks that do not form a trigraph | pass |
| Violation (must be reported) | trigraph in a string | pass |
| Violation (must be reported) | trigraph in code position | pass |

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

