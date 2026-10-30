# Rule 21.11

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports an include of the tgmath header.

Detection contract: Report an include of the tgmath header.

## Known limits

Only the include directive is examined.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | math.h | pass |
| Violation (must be reported) | tgmath.h | pass |
| Compliant (must stay silent) | complex.h is not tgmath | pass |
| Compliant (must stay silent) | a comment that mentions tgmath | pass |
| Violation (must be reported) | tgmath.h in angle brackets after other headers | pass |

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

### Compliant (must stay silent): complex.h is not tgmath

`main.c`

```c
#include <complex.h>
```

### Compliant (must stay silent): a comment that mentions tgmath

`main.c`

```c
/* tgmath.h is not used */
int x;
```

### Violation (must be reported): tgmath.h in angle brackets after other headers

`main.c`

```c
#include <math.h>
#include <tgmath.h> /*@V*/
```

