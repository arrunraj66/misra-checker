# Rule 21.12

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to a floating-point exception handling function from fenv.h (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: floating-point exception handling function from fenv.h called (matched by callee name).

## Known limits

The exception macros are not examined.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no fenv use | pass |
| Violation (must be reported) | clearing and testing exceptions | pass |

## Case sources

### Compliant (must stay silent): no fenv use

`main.c`

```c
double f(double x) { return x * 2.0; }
```

### Violation (must be reported): clearing and testing exceptions

`main.c`

```c
#include <fenv.h>
void f(void)
{
    (void)feclearexcept(FE_ALL_EXCEPT); /*@V*/
    (void)fetestexcept(FE_ALL_EXCEPT); /*@V*/
}
```

