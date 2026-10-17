# Rule 21.11

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports an include of the tgmath header.

Detection contract: Report an include of the tgmath header.

## Known limits

Only the include directive is examined.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | math.h | pass |
| Violation (must be reported) | tgmath.h | pass |

## Case sources

### Compliant (must stay silent): math.h

`main.c`

```c
#include <math.h>
double f(double x) { return sqrt(x); }
```

### Violation (must be reported): tgmath.h

`main.c`

```c
#include <tgmath.h> /*@V*/
double f(double x) { return x; }
```

