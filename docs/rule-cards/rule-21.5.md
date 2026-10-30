# Rule 21.5

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to signal or raise (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: signal handling function called (matched by callee name).

## Known limits

Other functions of the signal header are not covered.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no signal handling | pass |
| Violation (must be reported) | signal and raise | pass |
| Violation (must be reported) | raise | pass |
| Compliant (must stay silent) | variable named signal in a local scope | pass |
| Compliant (must stay silent) | the word raise inside a string | pass |

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

### Violation (must be reported): raise

`main.c`

```c
#include <signal.h>
void f(void)
{
    (void)raise(2); /*@V*/
}
```

### Compliant (must stay silent): variable named signal in a local scope

`main.c`

```c
int f(int signal) { return signal; }
```

### Compliant (must stay silent): the word raise inside a string

`main.c`

```c
const char *s = "raise";
```

