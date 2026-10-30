# Rule 21.5

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to signal or raise (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: signal handling function called (matched by callee name).

## Known limits

Other functions of the signal header are not covered.

- **False negative:** a signal constant used without calling a signal function (only the functions are followed) (documented limit)

## Tested cases

7 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no signal handling | pass |
| Violation (must be reported) | signal and raise | pass |
| Violation (must be reported) | raise | pass |
| Compliant (must stay silent) | variable named signal in a local scope | pass |
| Compliant (must stay silent) | the word raise inside a string | pass |
| Known false negative (not detected yet) | a signal constant used without calling a signal function (only the functions are followed) | documented limit |
| Compliant (must stay silent) | a struct member named raise | pass |
| Compliant (must stay silent) | a macro named like a signal constant but unrelated | pass |

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

### Known false negative (not detected yet): a signal constant used without calling a signal function (only the functions are followed)

`main.c`

```c
#include <signal.h>
int f(void)
{
    return SIGINT; /*@V*/
}
```

### Compliant (must stay silent): a struct member named raise

`main.c`

```c
struct ops { int raise; };
int f(struct ops *o) { return o->raise; }
```

### Compliant (must stay silent): a macro named like a signal constant but unrelated

`main.c`

```c
#define SIGNAL_COUNT 3
int n = SIGNAL_COUNT;
```

