# Rule 8.3

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Declarations and definitions

## What the detector reports

Reports declarations of one external name whose type (qualifiers included) or parameter names differ from the first declaration seen.

Detection contract: Report declarations of one external name whose type (qualifiers included) or parameter names differ from the first declaration seen.

## Known limits

Only declarations in the analysed translation unit are compared; conflicts with a declaration in a header that is not visible are not found.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | identical declarations | pass |
| Compliant (must stay silent) | declarations of different functions | pass |
| Violation (must be reported) | parameter names differ between declaration and definition | pass |
| Violation (must be reported) | a top-level parameter qualifier differs (the compiler accepts it) | pass |
| Violation (must be reported) | an extern object declared with different types in two files | pass |
| Compliant (must stay silent) | the same declaration repeated in two files through a header | pass |
| Violation (must be reported) | a parameter name differs in two prototypes | pass |

## Case sources

### Compliant (must stay silent): identical declarations

`main.c`

```c
int f(int a, int b);
int f(int a, int b) { return a + b; }
```

### Compliant (must stay silent): declarations of different functions

`main.c`

```c
int f(int a);
int g(int b);
```

### Violation (must be reported): parameter names differ between declaration and definition

`main.c`

```c
int f(int a);
int f(int b) { return b; } /*@V*/
```

### Violation (must be reported): a top-level parameter qualifier differs (the compiler accepts it)

`main.c`

```c
void put(const int count);
void put(int count) { } /*@V*/
```

### Violation (must be reported): an extern object declared with different types in two files

`a.c`

```c
int level = 1;
```

`b.c`

```c
extern long level; /*@V*/
long get(void) { return level; }
```

### Compliant (must stay silent): the same declaration repeated in two files through a header

`h.h`

```c
extern int level;
```

`a.c`

```c
#include "h.h"
int level = 1;
```

`b.c`

```c
#include "h.h"
int get(void) { return level; }
```

### Violation (must be reported): a parameter name differs in two prototypes

`main.c`

```c
int f(int a, int b);
int f(int a, int c); /*@V*/
```

