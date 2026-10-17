# Rule 21.5

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to signal or raise (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: signal handling function called (matched by callee name).

## Known limits

Other functions of the signal header are not covered.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no signal handling | pass |
| Violation (must be reported) | signal and raise | pass |

## Case sources

### Compliant (must stay silent): no signal handling

`main.c`

```c
void f(void) { }
```

### Violation (must be reported): signal and raise

`main.c`

```c
#include <signal.h>
void handler(int s) { (void)s; }
void f(void)
{
    signal(2, handler); /*@V*/
    raise(2); /*@V*/
}
```

