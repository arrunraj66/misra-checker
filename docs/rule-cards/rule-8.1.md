# Rule 8.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports a declaration written without a type specifier (implicit int).

Detection contract: Report a declaration with no written type specifier (implicit int).

## Known limits

Only declarations that the compiler accepts with a warning are reachable; strict compilers reject them as errors.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | explicit types | pass |
| Violation (must be reported) | object declared without a type | pass |
| Violation (must be reported) | function defined without a return type | pass |
| Violation (must be reported) | implicit int on a static object | pass |
| Violation (must be reported) | const without a type | pass |
| Compliant (must stay silent) | unsigned, long and short alone are explicit types | pass |
| Violation (must be reported) | main without a return type | pass |
| Compliant (must stay silent) | a typedef name used as the type | pass |
| Compliant (must stay silent) | a struct pointer declaration | pass |

## Case sources

### Compliant (must stay silent): explicit types

`main.c`

```c
static int a = 3;
int f(void) { return 0; }
```

### Violation (must be reported): object declared without a type

`main.c`

```c
static a = 3; /*@V*/
```

### Violation (must be reported): function defined without a return type

`main.c`

```c
f(void) { return 0; } /*@V*/
```

### Violation (must be reported): implicit int on a static object

`main.c`

```c
static counter = 0; /*@V*/
```

### Violation (must be reported): const without a type

`main.c`

```c
const limit = 4; /*@V*/
```

### Compliant (must stay silent): unsigned, long and short alone are explicit types

`main.c`

```c
unsigned a;
long b;
short c;
signed d;
```

### Violation (must be reported): main without a return type

`main.c`

```c
main(void) { return 0; } /*@V*/
```

### Compliant (must stay silent): a typedef name used as the type

`main.c`

```c
typedef int count_t;
count_t n;
```

### Compliant (must stay silent): a struct pointer declaration

`main.c`

```c
struct s { int a; };
struct s *p;
```

