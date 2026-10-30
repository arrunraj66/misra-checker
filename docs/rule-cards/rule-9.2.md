# Rule 9.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Initialization

## What the detector reports

Reports an aggregate initializer that omits the braces of a nested aggregate (the single {0} form is accepted).

Detection contract: Report an aggregate initializer that omits the braces of a nested aggregate (the {0} idiom is exempt).

## Known limits

Only initializer lists are examined.

## Tested cases

10 of 10 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | fully braced | pass |
| Compliant (must stay silent) | the zero initializer | pass |
| Compliant (must stay silent) | braces for a nested struct | pass |
| Violation (must be reported) | nested array initialized without inner braces | pass |
| Violation (must be reported) | nested struct initialized without braces | pass |
| Compliant (must stay silent) | structure with a nested array fully braced | pass |
| Violation (must be reported) | structure with a nested array not braced | pass |
| Compliant (must stay silent) | array of structures each braced | pass |
| Compliant (must stay silent) | string initialiser of a char member | pass |
| Compliant (must stay silent) | empty-style zero initialiser of nested aggregate | pass |

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

### Compliant (must stay silent): structure with a nested array fully braced

`main.c`

```c
struct s { int v[2]; int w; };
struct s a = { { 1, 2 }, 3 };
```

### Violation (must be reported): structure with a nested array not braced

`main.c`

```c
struct s { int v[2]; int w; };
struct s a = { 1, 2, 3 }; /*@V*/
```

### Compliant (must stay silent): array of structures each braced

`main.c`

```c
struct p { int x; int y; };
struct p t[2] = { { 1, 2 }, { 3, 4 } };
```

### Compliant (must stay silent): string initialiser of a char member

`main.c`

```c
struct s { char name[4]; int id; };
struct s a = { "ab", 1 };
```

### Compliant (must stay silent): empty-style zero initialiser of nested aggregate

`main.c`

```c
int m[2][2] = { { 0 } };
```

