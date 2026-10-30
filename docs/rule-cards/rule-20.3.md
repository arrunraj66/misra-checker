# Rule 20.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports an include whose operand is not written as a header name; the macro may still expand correctly, so the finding is only possible.

Detection contract: Report an include whose operand is not written as a header name; the macro may still expand correctly, so the finding is only possible.

## Known limits

Reported as a possible finding.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | written as header names | pass |
| Violation (must be reported) | the operand is a macro | pass |
| Compliant (must stay silent) | a header name followed by a comment | pass |
| Violation (must be reported) | a macro that expands to a quoted name | pass |
| Violation (must be reported) | a macro expanding to an angle name | pass |
| Compliant (must stay silent) | an include inside an inactive group written with a macro | pass |
| Violation (must be reported) | a macro that expands to a header in an elif branch that is taken | pass |

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

### Compliant (must stay silent): a header name followed by a comment

`main.c`

```c
#include <stddef.h> // the usual header
int x;
```

### Violation (must be reported): a macro that expands to a quoted name

`main.c`

```c
#define HDR "stddef.h"
#include HDR /*@V*/
```

### Violation (must be reported): a macro expanding to an angle name

`main.c`

```c
#define HDR <stddef.h>
#include HDR /*@V*/
```

### Compliant (must stay silent): an include inside an inactive group written with a macro

`main.c`

```c
#if 0
#include HEADER
#endif
int x;
```

### Violation (must be reported): a macro that expands to a header in an elif branch that is taken

`main.c`

```c
#define HDR <stddef.h>
#if 1
#include HDR /*@V*/
#endif
```

