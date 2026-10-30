# Rule 21.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to setjmp or longjmp (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: setjmp/longjmp facility used (matched by callee name).

## Known limits

setjmp is a macro in some libraries; it is found when the compiler expands it to a call.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no jumps | pass |
| Violation (must be reported) | longjmp | pass |
| Violation (must be reported) | setjmp | pass |
| Violation (must be reported) | longjmp through a jmp_buf | pass |
| Compliant (must stay silent) | a variable named setjmp_count | pass |

## Case sources

### Compliant (must stay silent): no jumps

`main.c`

```c
void f(void) { }
```

### Violation (must be reported): longjmp

`main.c`

```c
#include <setjmp.h>
jmp_buf env;
void f(void)
{
    longjmp(env, 1); /*@V*/
}
```

### Violation (must be reported): setjmp

`main.c`

```c
#include <setjmp.h>
jmp_buf env;
int g(void)
{
    return setjmp(env); /*@V*/
}
```

### Violation (must be reported): longjmp through a jmp_buf

`main.c`

```c
#include <setjmp.h>
jmp_buf env;
void f(void)
{
    longjmp(env, 1); /*@V*/
}
```

### Compliant (must stay silent): a variable named setjmp_count

`main.c`

```c
int setjmp_count;
```

