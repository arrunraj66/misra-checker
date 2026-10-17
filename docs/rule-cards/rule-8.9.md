# Rule 8.9

**Category:** Advisory · **Decidability:** Decidable · **Scope:** System · **Topic:** Declarations and definitions

## What the detector reports

Reports a file-scope static object that is referenced from exactly one function.

Detection contract: Report a file-scope static object that is referenced from exactly one function.

## Known limits

Counts references by function in the main file; an object used only at file scope or from several functions is accepted.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | static object used by two functions | pass |
| Compliant (must stay silent) | an object declared inside the function | pass |
| Violation (must be reported) | file-scope static used by one function only | pass |
| Violation (must be reported) | file-scope static constant used by one function | pass |
| Compliant (must stay silent) | a static used in two functions | pass |
| Compliant (must stay silent) | two statics each used by two functions | pass |
| Violation (must be reported) | a static array used by one function | pass |

## Case sources

### Compliant (must stay silent): static object used by two functions

`main.c`

```c
static int shared;
void set(int v) { shared = v; }
int get(void) { return shared; }
```

### Compliant (must stay silent): an object declared inside the function

`main.c`

```c
int f(void) { static int calls; calls++; return calls; }
```

### Violation (must be reported): file-scope static used by one function only

`main.c`

```c
static int counter; /*@V*/
int next(void) { counter++; return counter; }
```

### Violation (must be reported): file-scope static constant used by one function

`main.c`

```c
static const int limit = 5; /*@V*/
int clamp(int v) { return v > limit ? limit : v; }
```

### Compliant (must stay silent): a static used in two functions

`main.c`

```c
static int state;
void a(void) { state = 1; }
int b(void) { return state; }
```

### Compliant (must stay silent): two statics each used by two functions

`main.c`

```c
static int base = 4;
static int derived = 5;
int f(void) { return base + derived; }
int g(void) { return base - derived; }
```

### Violation (must be reported): a static array used by one function

`main.c`

```c
static int buf[4]; /*@V*/
int f(int i) { buf[i % 4] = i; return buf[0]; }
```

