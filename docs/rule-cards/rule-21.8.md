# Rule 21.8

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to abort, exit, getenv or system (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: abort, exit, getenv or system called (matched by callee name).

## Known limits

_Exit and quick_exit are not covered.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no calls | pass |
| Violation (must be reported) | exit and abort | pass |
| Violation (must be reported) | getenv and system | pass |
| Violation (must be reported) | system | pass |
| Compliant (must stay silent) | a local named exit_code | pass |

## Case sources

### Compliant (must stay silent): no calls

`main.c`

```c
int f(void) { return 0; }
```

### Violation (must be reported): exit and abort

`main.c`

```c
#include <stdlib.h>
void f(void)
{
    abort(); /*@V*/
}
void g(void)
{
    exit(1); /*@V*/
}
```

### Violation (must be reported): getenv and system

`main.c`

```c
#include <stdlib.h>
void h(void)
{
    (void)getenv("A"); /*@V*/
    (void)system("ls"); /*@V*/
}
```

### Violation (must be reported): system

`main.c`

```c
#include <stdlib.h>
int f(void) { return system("ls"); } /*@V*/
```

### Compliant (must stay silent): a local named exit_code

`main.c`

```c
int f(void) { int exit_code = 0; return exit_code; }
```

