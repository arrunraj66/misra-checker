# Rule 8.7

**Category:** Advisory · **Decidability:** Decidable · **Scope:** System · **Topic:** Declarations and definitions

## What the detector reports

Whole-program check: reports an externally visible function or object that is defined but referenced only from its own translation unit (main is exempt).

Detection contract: Whole-program: report an externally visible function or object that is defined but referenced only from its own translation unit (main excluded; Inconclusive with a single translation unit).

## Known limits

Needs every translation unit of the program in one run.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | external function used from another file | pass |
| Compliant (must stay silent) | main is exempt | pass |
| Violation (must be reported) | external function used only in its own file | pass |
| Violation (must be reported) | external object used only in its own file | pass |
| Violation (must be reported) | a header declaration that no other file uses | pass |
| Compliant (must stay silent) | an object accessed from two files | pass |
| Violation (must be reported) | an external function used by one file only | pass |
| Compliant (must stay silent) | an external function used by both files | pass |

## Case sources

### Compliant (must stay silent): external function used from another file

`a.c`

```c
int shared(void) { return 1; }
```

`b.c`

```c
int shared(void);
int main(void) { return shared(); }
```

### Compliant (must stay silent): main is exempt

`main.c`

```c
int main(void) { return 0; }
```

### Violation (must be reported): external function used only in its own file

`a.c`

```c
int only_here(void) { return 1; } /*@V*/
int caller(void) { return only_here(); }
```

`b.c`

```c
int caller(void);
int top(void) { return caller(); }
```

### Violation (must be reported): external object used only in its own file

`a.c`

```c
int only_obj = 1; /*@V*/
int get(void) { return only_obj; }
```

`b.c`

```c
int get(void);
int top(void) { return get(); }
```

### Violation (must be reported): a header declaration that no other file uses

`h.h`

```c
int only_here(void);
```

`a.c`

```c
#include "h.h"
int only_here(void) { return 1; } /*@V*/
int run(void) { return only_here(); }
```

`b.c`

```c
int other(void) { return 2; }
```

### Compliant (must stay silent): an object accessed from two files

`a.c`

```c
int shared = 1;
int get(void);
int main(void) { return get(); }
```

`b.c`

```c
extern int shared;
int get(void) { return shared; }
```

### Violation (must be reported): an external function used by one file only

`a.c`

```c
int helper(void) { return 1; } /*@V*/
int main(void) { return helper(); }
```

`b.c`

```c
int other(void) { return 2; }
```

### Compliant (must stay silent): an external function used by both files

`a.c`

```c
int shared(void);
int main(void) { return shared(); }
```

`b.c`

```c
int shared(void) { return 1; }
```

