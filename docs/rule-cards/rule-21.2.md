# Rule 21.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a declaration that uses a reserved identifier (leading underscore forms) or defines a function that has the name of a standard library function.

Detection contract: Report a declaration using an underscore-reserved identifier or defining a standard library function name.

## Known limits

Only the underscore forms and the standard function names listed in the checker are covered.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary names | pass |
| Compliant (must stay silent) | a local name with a leading underscore and a lowercase letter is not reserved | pass |
| Violation (must be reported) | file-scope names with leading underscores | pass |
| Violation (must be reported) | defining a standard library function | pass |
| Violation (must be reported) | a static function named like a library function | pass |
| Compliant (must stay silent) | a prototype from the library header | pass |
| Violation (must be reported) | a tag with a leading underscore at file scope | pass |
| Compliant (must stay silent) | a local variable named like a library function | pass |

## Case sources

### Compliant (must stay silent): ordinary names

`main.c`

```c
int count;
int my_strlen(const char *s) { int n = 0; while (s[n] != 0) { n++; } return n; }
```

### Compliant (must stay silent): a local name with a leading underscore and a lowercase letter is not reserved

`main.c`

```c
void f(void) { int _local = 1; (void)_local; }
```

### Violation (must be reported): file-scope names with leading underscores

`main.c`

```c
int __internal; /*@V*/
int _Hidden; /*@V*/
```

### Violation (must be reported): defining a standard library function

`main.c`

```c
unsigned long strlen(const char *s) { return 0; } /*@V*/
```

### Violation (must be reported): a static function named like a library function

`main.c`

```c
static int abs(int a) { return a; } /*@V*/
int g(void) { return abs(1); }
```

### Compliant (must stay silent): a prototype from the library header

`main.c`

```c
#include <string.h>
unsigned long f(const char *s) { return strlen(s); }
```

### Violation (must be reported): a tag with a leading underscore at file scope

`main.c`

```c
struct _node { int a; }; /*@V*/
struct _node n;
```

### Compliant (must stay silent): a local variable named like a library function

`main.c`

```c
int f(void) { int time = 1; return time; }
```

