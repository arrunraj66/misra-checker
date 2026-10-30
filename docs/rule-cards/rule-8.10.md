# Rule 8.10

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports an inline function definition that is not declared static.

Detection contract: Report an inline function definition that is not declared static.

## Known limits

Only inline function definitions are examined.

- **False negative:** an inline function declaration without static (only definitions are examined) (documented limit)

## Tested cases

6 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | static inline | pass |
| Compliant (must stay silent) | a plain function | pass |
| Violation (must be reported) | inline without static | pass |
| Violation (must be reported) | extern inline definition | pass |
| Compliant (must stay silent) | a declaration only of a static inline function | pass |
| Compliant (must stay silent) | a static inline function used twice | pass |
| Known false negative (not detected yet) | an inline function declaration without static (only definitions are examined) | documented limit |

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

### Violation (must be reported): extern inline definition

`main.c`

```c
extern inline int sq(int a) { return a * a; } /*@V*/
```

### Compliant (must stay silent): a declaration only of a static inline function

`main.c`

```c
static inline int sq(int a);
static inline int sq(int a) { return a * a; }
```

### Compliant (must stay silent): a static inline function used twice

`main.c`

```c
static inline int sq(int a) { return a * a; }
int f(int x) { return sq(x) + sq(x + 1); }
```

### Known false negative (not detected yet): an inline function declaration without static (only definitions are examined)

`main.c`

```c
inline int sq(int a); /*@V*/
```

