# Rule 17.6

**Category:** Mandatory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Functions

## What the detector reports

Reports an array parameter declared with the static keyword inside the brackets.

Detection contract: Report an array parameter declared with the static keyword inside the brackets.

## Known limits

Only the bracketed form is examined.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary array parameter | pass |
| Violation (must be reported) | static inside the brackets | pass |
| Violation (must be reported) | in a definition | pass |

## Case sources

### Compliant (must stay silent): ordinary array parameter

`main.c`

```c
void f(int a[4]);
void g(int b[]);
```

### Violation (must be reported): static inside the brackets

`main.c`

```c
void f(int a[static 4]); /*@V*/
```

### Violation (must be reported): in a definition

`main.c`

```c
int sum(int a[static 2]) { return a[0] + a[1]; } /*@V*/
```

