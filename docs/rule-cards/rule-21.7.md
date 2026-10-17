# Rule 21.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to atof, atoi, atol or atoll (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: string-to-number conversion from the atox family called (matched by callee name).

## Known limits

strtol and friends are not reported.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | strtol | pass |
| Violation (must be reported) | the ato family | pass |

## Case sources

### Compliant (must stay silent): strtol

`main.c`

```c
#include <stdlib.h>
long f(const char *s) { return strtol(s, 0, 10); }
```

### Violation (must be reported): the ato family

`main.c`

```c
#include <stdlib.h>
void f(const char *s)
{
    (void)atoi(s); /*@V*/
    (void)atol(s); /*@V*/
    (void)atof(s); /*@V*/
}
```

