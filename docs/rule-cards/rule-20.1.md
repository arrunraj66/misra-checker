# Rule 20.1

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports an include directive that is preceded by anything other than preprocessor directives or comments.

Detection contract: Report an include directive preceded by anything other than directives or comments.

## Known limits

Directives in inactive conditional groups are not treated as code or as includes.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | includes first, after comments and directives | pass |
| Compliant (must stay silent) | an include in an inactive group | pass |
| Violation (must be reported) | an include after a declaration | pass |
| Violation (must be reported) | an include after a function | pass |
| Violation (must be reported) | an include inside a function body | pass |
| Compliant (must stay silent) | pragma, define and error-free directives before the includes | pass |
| Violation (must be reported) | an include after a stray semicolon declaration | pass |

## Case sources

### Compliant (must stay silent): includes first, after comments and directives

`main.c`

```c
/* header comment */
#define LOCAL 1
#include <stddef.h>
#include <string.h>
int a;
```

### Compliant (must stay silent): an include in an inactive group

`main.c`

```c
#ifdef NEVER_DEFINED_FOR_THIS_TEST
    #include "missing_header.h"
#endif
int a;
```

### Violation (must be reported): an include after a declaration

`main.c`

```c
int a;
#include <stddef.h> /*@V*/
```

### Violation (must be reported): an include after a function

`main.c`

```c
void f(void) { }
#include <string.h> /*@V*/
```

### Violation (must be reported): an include inside a function body

`main.c`

```c
void f(void)
{
#include "inner.h" /*@V*/
}
```

### Compliant (must stay silent): pragma, define and error-free directives before the includes

`main.c`

```c
#pragma once
#define LEVEL 2
#if LEVEL > 1
#include <stddef.h>
#endif
#include <stdint.h>
int x;
```

### Violation (must be reported): an include after a stray semicolon declaration

`main.c`

```c
int x;
#include <stddef.h> /*@V*/
```

