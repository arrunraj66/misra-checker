# Rule 9.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Initialization

## What the detector reports

Reports an element that is designated more than once in one initializer list.

Detection contract: Report an element designated more than once in one initializer list.

## Known limits

Only designated initializers are examined.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | distinct designators | pass |
| Violation (must be reported) | the same array element twice | pass |
| Violation (must be reported) | the same member twice | pass |

## Case sources

### Compliant (must stay silent): distinct designators

`main.c`

```c
int a[3] = { [0] = 1, [2] = 3 };
struct s { int x; int y; } v = { .x = 1, .y = 2 };
```

### Violation (must be reported): the same array element twice

`main.c`

```c
int a[3] = { [1] = 1, [1] = 2 }; /*@V*/
```

### Violation (must be reported): the same member twice

`main.c`

```c
struct s { int x; int y; };
struct s v = { .x = 1, .x = 2 }; /*@V*/
```

