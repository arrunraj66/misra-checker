# Rule 8.5

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Declarations and definitions

## What the detector reports

Reports an external function or object that has non-defining declarations in more than one file: declare it once, in a header that both the definition and the users include.

Detection contract: Report an external function or object that has non-defining declarations in more than one file (declare once, in a header).

## Known limits

Compares the files in which the declarations are written; it needs the whole program in one run to see more than one user.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | one header declares it, everyone includes the header | pass |
| Violation (must be reported) | two source files each declare the same external object | pass |

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

