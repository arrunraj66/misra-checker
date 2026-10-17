# Rule 20.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports an include whose operand is not written as a header name; the macro may still expand correctly, so the finding is only possible.

Detection contract: Report an include whose operand is not written as a header name; the macro may still expand correctly, so the finding is only possible.

## Known limits

Reported as a possible finding.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | written as header names | pass |
| Violation (must be reported) | the operand is a macro | pass |

## Case sources

### Compliant (must stay silent): written as header names

`main.c`

```c
#include <stddef.h>
#include "stddef.h"
```

### Violation (must be reported): the operand is a macro

`main.c`

```c
#define HEADER <stddef.h>
#include HEADER /*@V*/
```

