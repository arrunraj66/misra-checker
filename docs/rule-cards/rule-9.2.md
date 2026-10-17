# Rule 9.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Initialization

## What the detector reports

Reports an aggregate initializer that omits the braces of a nested aggregate (the single {0} form is accepted).

Detection contract: Report an aggregate initializer that omits the braces of a nested aggregate (the {0} idiom is exempt).

## Known limits

Only initializer lists are examined.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | fully braced | pass |
| Compliant (must stay silent) | the zero initializer | pass |
| Compliant (must stay silent) | braces for a nested struct | pass |
| Violation (must be reported) | nested array initialized without inner braces | pass |
| Violation (must be reported) | nested struct initialized without braces | pass |

## Case sources

### Compliant (must stay silent): fully braced

`main.c`

```c
int a[2][2] = { { 1, 2 }, { 3, 4 } };
```

### Compliant (must stay silent): the zero initializer

`main.c`

```c
int b[2][2] = { 0 };
struct pair { int x; int y; } p = { 0 };
```

### Compliant (must stay silent): braces for a nested struct

`main.c`

```c
struct inner { int a; int b; };
struct outer { struct inner i; int c; };
struct outer o = { { 1, 2 }, 3 };
```

### Violation (must be reported): nested array initialized without inner braces

`main.c`

```c
int a[2][2] = { 1, 2, 3, 4 }; /*@V*/
```

### Violation (must be reported): nested struct initialized without braces

`main.c`

```c
struct inner { int a; int b; };
struct outer { struct inner i; int c; };
struct outer o = { 1, 2, 3 }; /*@V*/
```

