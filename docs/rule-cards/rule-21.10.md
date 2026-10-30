# Rule 21.10

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to a standard time and date function (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: standard time/date function called (matched by callee name).

## Known limits

Only the functions of time.h that read or format the time are covered.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no time functions | pass |
| Violation (must be reported) | time, clock and strftime | pass |
| Violation (must be reported) | difftime and mktime | pass |
| Compliant (must stay silent) | a user function called clock_tick | pass |
| Violation (must be reported) | time | pass |
| Violation (must be reported) | localtime | pass |
| Compliant (must stay silent) | a variable named time | pass |
| Compliant (must stay silent) | a struct tm declared without calling any time function | pass |

## Case sources

### Compliant (must stay silent): no time functions

`main.c`

```c
int f(int x) { return x; }
```

### Violation (must be reported): time, clock and strftime

`main.c`

```c
#include <time.h>
void f(char *buf)
{
    time_t t = time(0); /*@V*/
    (void)clock(); /*@V*/
    (void)strftime(buf, 8, "%H", localtime(&t)); /*@V*/
}
```

### Violation (must be reported): difftime and mktime

`main.c`

```c
#include <time.h>
double f(time_t a, time_t b) { return difftime(a, b); } /*@V*/
```

### Compliant (must stay silent): a user function called clock_tick

`main.c`

```c
void clock_tick(void);
void f(void) { clock_tick(); }
```

### Violation (must be reported): time

`main.c`

```c
#include <time.h>
time_t f(void) { return time(0); } /*@V*/
```

### Violation (must be reported): localtime

`main.c`

```c
#include <time.h>
void f(const time_t *t)
{
    (void)localtime(t); /*@V*/
}
```

### Compliant (must stay silent): a variable named time

`main.c`

```c
int f(void)
{
    int time = 3;
    return time;
}
```

### Compliant (must stay silent): a struct tm declared without calling any time function

`main.c`

```c
#include <time.h>
struct tm g;
```

