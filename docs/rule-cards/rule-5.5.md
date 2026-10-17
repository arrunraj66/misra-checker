# Rule 5.5

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Identifiers

## What the detector reports

Reports a declared identifier that has the same name as a macro defined in the main file.

Detection contract: Report a declared identifier that has the same name as a macro defined in the main file.

## Known limits

A macro defined in a header is not compared with identifiers of the main file.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | macro and identifier with different names | pass |
| Compliant (must stay silent) | an unrelated macro defined after the declarations | pass |
| Violation (must be reported) | identifier declared before a macro of the same name | pass |
| Violation (must be reported) | a function declared before a function-like macro of the same name | pass |

## Case sources

### Compliant (must stay silent): macro and identifier with different names

`main.c`

```c
#define LIMIT 10
int count[LIMIT];
```

### Compliant (must stay silent): an unrelated macro defined after the declarations

`main.c`

```c
int level;
#define OTHER 3
int v = OTHER;
```

### Violation (must be reported): identifier declared before a macro of the same name

`main.c`

```c
int level; /*@V*/
#define level_unused 1
#define level 3
int v = 0;
```

### Violation (must be reported): a function declared before a function-like macro of the same name

`main.c`

```c
int maxi(int a, int b); /*@V*/
#define maxi(a, b) (((a) > (b)) ? (a) : (b))
int use(int x) { return maxi(x, 1); }
```

