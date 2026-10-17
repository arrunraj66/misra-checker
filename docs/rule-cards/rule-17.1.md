# Rule 17.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Functions

## What the detector reports

Reports use of va_arg and of the va_start, va_end and va_copy built-ins from the variable-argument facility.

Detection contract: Report use of va_arg or the va_start/va_end/va_copy built-ins.

## Known limits

Matched by use; a function declared with an ellipsis but never reading its arguments is not reported.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a normal function | pass |
| Compliant (must stay silent) | only the header is included | pass |
| Violation (must be reported) | reading variable arguments | pass |

## Case sources

### Compliant (must stay silent): a normal function

`main.c`

```c
int add(int a, int b) { return a + b; }
```

### Compliant (must stay silent): only the header is included

`main.c`

```c
#include <stdarg.h>
int plain(int a) { return a; }
```

### Violation (must be reported): reading variable arguments

`main.c`

```c
#include <stdarg.h>
int sum(int n, ...)
{
    va_list ap;
    int v;
    va_start(ap, n); /*@V*/
    v = va_arg(ap, int); /*@V*/
    va_end(ap); /*@V*/
    return v;
}
```

