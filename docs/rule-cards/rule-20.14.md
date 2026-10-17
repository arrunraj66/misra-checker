# Rule 20.14

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a conditional directive that is not closed in the file where it was opened.

Detection contract: Report a conditional directive that is not closed in the file where it was opened (the compiler rejects these).

## Known limits

The compiler rejects these files; the finding comes from that diagnostic.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | balanced conditionals | pass |
| Violation (must be reported) | an #if without #endif | pass |
| Violation (must be reported) | an #ifdef without #endif inside a function body file | pass |

## Case sources

### Compliant (must stay silent): balanced conditionals

`main.c`

```c
#ifdef A
int a;
#else
int b;
#endif
```

### Violation (must be reported): an #if without #endif

`main.c`

```c
#if 1
int a;
```

### Violation (must be reported): an #ifdef without #endif inside a function body file

`main.c`

```c
#ifdef A
void f(void) { }
```

