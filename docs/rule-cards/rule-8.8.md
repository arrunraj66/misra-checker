# Rule 8.8

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports a redeclaration of an internal-linkage function or object that omits the static specifier.

Detection contract: Report a redeclaration of an internal-linkage function or object that omits the static specifier.

## Known limits

Only redeclarations in the same translation unit are compared.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | static on every declaration | pass |
| Compliant (must stay silent) | a different function without static | pass |
| Violation (must be reported) | the definition omits static after a static declaration | pass |
| Violation (must be reported) | an extern redeclaration of a static function | pass |

## Case sources

### Compliant (must stay silent): static on every declaration

`main.c`

```c
static int f(void);
static int f(void) { return 1; }
```

### Compliant (must stay silent): a different function without static

`main.c`

```c
int g(void);
```

### Violation (must be reported): the definition omits static after a static declaration

`main.c`

```c
static int f(void);
int f(void) { return 1; } /*@V*/
```

### Violation (must be reported): an extern redeclaration of a static function

`main.c`

```c
static int g(void);
extern int g(void) { return 1; } /*@V*/
```

