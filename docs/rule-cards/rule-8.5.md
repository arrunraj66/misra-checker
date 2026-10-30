# Rule 8.5

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Declarations and definitions

## What the detector reports

Reports an external function or object that has non-defining declarations in more than one file: declare it once, in a header that both the definition and the users include.

Detection contract: Report an external function or object that has non-defining declarations in more than one file (declare once, in a header).

## Known limits

Compares the files in which the declarations are written; it needs the whole program in one run to see more than one user.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | one header declares it, everyone includes the header | pass |
| Violation (must be reported) | two source files each declare the same external object | pass |
| Violation (must be reported) | a function declared in two source files | pass |
| Compliant (must stay silent) | a declaration in a header repeated by the defining file only | pass |
| Compliant (must stay silent) | an extern declaration of a function in one source file only | pass |

## Case sources

### Compliant (must stay silent): one header declares it, everyone includes the header

`common.h`

```c
extern int shared;
```

`a.c`

```c
#include "common.h"
int shared = 1;
```

`b.c`

```c
#include "common.h"
int read_shared(void) { return shared; }
```

### Violation (must be reported): two source files each declare the same external object

`a.c`

```c
int shared = 1;
extern int unused_decl_a;
extern int declared_twice; /*@V*/
```

`b.c`

```c
extern int declared_twice; /*@V*/
int read_twice(void) { return declared_twice; }
```

`c.c`

```c
int declared_twice = 0;
```

### Violation (must be reported): a function declared in two source files

`a.c`

```c
int helper(void);
int run(void) { return helper(); }
```

`b.c`

```c
int helper(void); /*@V*/
int helper(void) { return 1; }
```

### Compliant (must stay silent): a declaration in a header repeated by the defining file only

`h.h`

```c
extern int level;
```

`a.c`

```c
#include "h.h"
int level = 1;
```

`b.c`

```c
#include "h.h"
int get(void) { return level; }
```

### Compliant (must stay silent): an extern declaration of a function in one source file only

`a.c`

```c
int helper(void);
int run(void) { return helper(); }
```

`b.c`

```c
int helper(void) { return 1; }
```

