# Rule 8.10

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports an inline function definition that is not declared static.

Detection contract: Report an inline function definition that is not declared static.

## Known limits

Only inline function definitions are examined.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | static inline | pass |
| Compliant (must stay silent) | a plain function | pass |
| Violation (must be reported) | inline without static | pass |

## Case sources

### Compliant (must stay silent): static inline

`main.c`

```c
static inline int twice(int x) { return x * 2; }
int use(int v) { return twice(v); }
```

### Compliant (must stay silent): a plain function

`main.c`

```c
int plain(int x) { return x; }
```

### Violation (must be reported): inline without static

`main.c`

```c
inline int twice(int x) { return x * 2; } /*@V*/
```

