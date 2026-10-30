# Rule 20.5

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports every #undef directive.

Detection contract: Report every #undef directive.

## Known limits

None; every #undef is reported.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no #undef | pass |
| Violation (must be reported) | #undef of a macro | pass |
| Violation (must be reported) | #undef of a name that was never defined | pass |
| Violation (must be reported) | #undef inside a function | pass |
| Compliant (must stay silent) | only #define and #if | pass |
| Violation (must be reported) | #undef followed by redefinition | pass |
| Violation (must be reported) | #undef of a macro defined in an included header | pass |
| Compliant (must stay silent) | no #undef anywhere | pass |

## Case sources

### Compliant (must stay silent): no #undef

`main.c`

```c
#define LIMIT 10
int a[LIMIT];
```

### Violation (must be reported): #undef of a macro

`main.c`

```c
#define LIMIT 10
int a[LIMIT];
#undef LIMIT /*@V*/
```

### Violation (must be reported): #undef of a name that was never defined

`main.c`

```c
#undef NEVER_DEFINED /*@V*/
```

### Violation (must be reported): #undef inside a function

`main.c`

```c
#define LEVEL 1
void f(void)
{
#undef LEVEL /*@V*/
}
```

### Compliant (must stay silent): only #define and #if

`main.c`

```c
#define LEVEL 1
#if LEVEL
int x;
#endif
```

### Violation (must be reported): #undef followed by redefinition

`main.c`

```c
#define LEVEL 1
#undef LEVEL /*@V*/
#define LEVEL 2
int x = LEVEL;
```

### Violation (must be reported): #undef of a macro defined in an included header

`h.h`

```c
#define LIMIT 4
```

`main.c`

```c
#include "h.h"
#undef LIMIT /*@V*/
int x;
```

### Compliant (must stay silent): no #undef anywhere

`main.c`

```c
#define A 1
#define B (A + 1)
int x = B;
```

