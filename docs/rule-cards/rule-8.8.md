# Rule 8.8

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports a redeclaration of an internal-linkage function or object that omits the static specifier.

Detection contract: Report a redeclaration of an internal-linkage function or object that omits the static specifier.

## Known limits

Only redeclarations in the same translation unit are compared.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | static on every declaration | pass |
| Compliant (must stay silent) | a different function without static | pass |
| Violation (must be reported) | the definition omits static after a static declaration | pass |
| Violation (must be reported) | an extern redeclaration of a static function | pass |
| Violation (must be reported) | an extern redeclaration of a static object | pass |
| Compliant (must stay silent) | static function declared and defined static | pass |
| Violation (must be reported) | static declared after an extern declaration is missing on the definition | pass |
| Compliant (must stay silent) | static on both declaration and definition | pass |

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

### Violation (must be reported): an extern redeclaration of a static object

`main.c`

```c
static int level;
extern int level; /*@V*/
```

### Compliant (must stay silent): static function declared and defined static

`main.c`

```c
static int f(void);
static int f(void) { return 1; }
int g(void) { return f(); }
```

### Violation (must be reported): static declared after an extern declaration is missing on the definition

`main.c`

```c
static int f(void);
int f(void) { return 1; } /*@V*/
```

### Compliant (must stay silent): static on both declaration and definition

`main.c`

```c
static int f(void);
static int f(void) { return 1; }
```

