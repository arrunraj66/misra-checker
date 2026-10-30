# Rule 20.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports an include header name that contains a quote, a backslash, a comment marker, or (angle form) a double quote.

Detection contract: Report an include header name containing a quote, backslash, comment marker or (angle form) double quote.

## Known limits

The check is textual, so the sequence is reported even when it sits in an inactive group.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary header names | pass |
| Violation (must be reported) | a quote inside the name | pass |
| Violation (must be reported) | a backslash inside the name | pass |
| Violation (must be reported) | a comment marker inside the name | pass |
| Compliant (must stay silent) | a path with a directory separator | pass |
| Violation (must be reported) | an apostrophe inside an angle bracket header name | pass |
| Violation (must be reported) | a double-slash comment marker inside the name | pass |

## Case sources

### Compliant (must stay silent): ordinary header names

`main.c`

```c
#include <stddef.h>
#include "local.h"
```

### Violation (must be reported): a quote inside the name

`main.c`

```c
#include "a'b.h" /*@V*/
```

### Violation (must be reported): a backslash inside the name

`main.c`

```c
#include "dir\file.h" /*@V*/
```

### Violation (must be reported): a comment marker inside the name

`main.c`

```c
#include <a/*b.h> /*@V*/
```

### Compliant (must stay silent): a path with a directory separator

`main.c`

```c
#include <sys/types.h>
```

### Violation (must be reported): an apostrophe inside an angle bracket header name

`main.c`

```c
#include <a'b.h> /*@V*/
```

### Violation (must be reported): a double-slash comment marker inside the name

`main.c`

```c
#include "a//b.h" /*@V*/
```

