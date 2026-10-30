# Rule 20.14

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a conditional directive that is not closed in the file where it was opened.

Detection contract: Report a conditional directive that is not closed in the file where it was opened (the compiler rejects these).

## Known limits

The compiler rejects these files; the finding comes from that diagnostic.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | balanced conditionals | pass |
| Violation (must be reported) | an #if without #endif | pass |
| Violation (must be reported) | an #ifdef without #endif inside a function body file | pass |
| Violation (must be reported) | an #if without #endif in an included header | pass |
| Compliant (must stay silent) | nested balanced conditionals | pass |
| Violation (must be reported) | #else without a matching end in a header | pass |
| Compliant (must stay silent) | a header with balanced groups | pass |

## Case sources

### Compliant (must stay silent): balanced conditionals

`main.c`

```c
#ifdef A
int a;
#else
int b;
#endif
```

### Violation (must be reported): an #if without #endif

`main.c`

```c
#if 1
int a;
```

### Violation (must be reported): an #ifdef without #endif inside a function body file

`main.c`

```c
#ifdef A
void f(void) { }
```

### Violation (must be reported): an #if without #endif in an included header

`h.h`

```c
#if 1
int h;
```

`main.c`

```c
#include "h.h"
int a;
```

### Compliant (must stay silent): nested balanced conditionals

`main.c`

```c
#if 1
#ifdef A
int a;
#else
int b;
#endif
#endif
```

### Violation (must be reported): #else without a matching end in a header

`h.h`

```c
#ifdef A
int a;
#else
int b;
```

`main.c`

```c
#include "h.h"
int c;
```

### Compliant (must stay silent): a header with balanced groups

`h.h`

```c
#ifndef H_H
#define H_H
int a;
#endif
```

`main.c`

```c
#include "h.h"
int c;
```

