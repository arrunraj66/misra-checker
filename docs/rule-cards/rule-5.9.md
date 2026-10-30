# Rule 5.9

**Category:** Advisory · **Decidability:** Decidable · **Scope:** System · **Topic:** Identifiers

## What the detector reports

Whole-program check: reports internal-linkage functions or objects that share a name but are defined at different places in the program.

Detection contract: Whole-program: report internal-linkage functions or objects that share a name but are defined at different locations (Inconclusive with a single translation unit).

## Known limits

Needs every translation unit of the program in one run; with a single translation unit the rule is not assessed.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | static names that are distinct across files | pass |
| Violation (must be reported) | the same static name in two files | pass |
| Violation (must be reported) | the same static object name in two files | pass |
| Compliant (must stay silent) | a static inline function in a shared header is one declaration | pass |
| Violation (must be reported) | static functions of the same name with different bodies in two files | pass |
| Compliant (must stay silent) | a static function defined once and a different name elsewhere | pass |
| Violation (must be reported) | a static object of the same name in two files | pass |

## Case sources

### Compliant (must stay silent): static names that are distinct across files

`a.c`

```c
static int helper_a(void) { return 1; }
int use_a(void) { return helper_a(); }
```

`b.c`

```c
static int helper_b(void) { return 2; }
int use_b(void) { return helper_b(); }
```

### Violation (must be reported): the same static name in two files

`a.c`

```c
static int helper(void) { return 1; } /*@V*/
int use_a(void) { return helper(); }
```

`b.c`

```c
static int helper(void) { return 2; } /*@V*/
int use_b(void) { return helper(); }
```

### Violation (must be reported): the same static object name in two files

`a.c`

```c
static int state = 1; /*@V*/
int get_a(void) { return state; }
```

`b.c`

```c
static int state = 2; /*@V*/
int get_b(void) { return state; }
```

### Compliant (must stay silent): a static inline function in a shared header is one declaration

`util.h`

```c
static inline int twice(int a) { return a * 2; }
```

`a.c`

```c
#include "util.h"
int fa(int x) { return twice(x); }
```

`b.c`

```c
#include "util.h"
int fb(int x) { return twice(x) + 1; }
```

### Violation (must be reported): static functions of the same name with different bodies in two files

`a.c`

```c
static int step(int a) { return a + 1; }
int fa(int x) { return step(x); }
```

`b.c`

```c
static int step(int a) { return a + 2; } /*@V*/
int fb(int x) { return step(x); }
```

### Compliant (must stay silent): a static function defined once and a different name elsewhere

`a.c`

```c
static int step_a(int a) { return a + 1; }
int fa(int x) { return step_a(x); }
```

`b.c`

```c
static int step_b(int a) { return a + 2; }
int fb(int x) { return step_b(x); }
```

### Violation (must be reported): a static object of the same name in two files

`a.c`

```c
static int level = 1;
int ga(void) { return level; }
```

`b.c`

```c
static int level = 2; /*@V*/
int gb(void) { return level; }
```

