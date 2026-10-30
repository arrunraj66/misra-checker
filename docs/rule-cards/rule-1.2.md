# Rule 1.2

**Category:** Advisory · **Decidability:** Undecidable · **Scope:** SingleTranslationUnit · **Topic:** Standard C conformance

## What the detector reports

Reports language extensions: statement expressions, inline assembly, computed goto and label addresses, case ranges, typeof, the omitted middle operand of ?:, and explicit attributes.

Detection contract: Report use of compiler extensions: statement expressions, inline assembly, computed goto, label addresses, case ranges, typeof, omitted conditional operand and explicit attributes.

## Known limits

Compiler built-in functions and predefined macros are not reported. Only the extensions listed are detected.

## Tested cases

21 of 21 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | standard C | pass |
| Violation (must be reported) | statement expression | pass |
| Violation (must be reported) | inline assembly | pass |
| Violation (must be reported) | computed goto and label address | pass |
| Violation (must be reported) | case range | pass |
| Violation (must be reported) | typeof (the reserved spelling works in strict C99) | pass |
| Violation (must be reported) | typeof (the plain spelling is a keyword only in GNU mode) | pass |
| Violation (must be reported) | omitted middle operand of the conditional operator | pass |
| Violation (must be reported) | explicit attribute | pass |
| Compliant (must stay silent) | a function-like macro is not an extension | pass |
| Violation (must be reported) | attribute on a variable | pass |
| Violation (must be reported) | attribute on a function declaration | pass |
| Violation (must be reported) | binary literal | pass |
| Violation (must be reported) | zero-length array | pass |
| Compliant (must stay silent) | flexible array member is C99 | pass |
| Compliant (must stay silent) | restrict, inline, compound literal and long long are C99 | pass |
| Compliant (must stay silent) | designated initializers are C99 | pass |
| Compliant (must stay silent) | a comment mentioning __attribute__ and asm | pass |
| Violation (must be reported) | statement expression inside a macro use | pass |
| Violation (must be reported) | inline assembly with volatile | pass |
| Violation (must be reported) | case range in a switch with several labels | pass |

## Case sources

### Compliant (must stay silent): standard C

`main.c`

```c
static inline int twice(int x) { return x * 2; }
int f(int v) { return v ? twice(v) : 0; }
```

### Violation (must be reported): statement expression

`main.c`

```c
int f(void) { return ({ int t = 1; t; }); } /*@V*/
```

### Violation (must be reported): inline assembly

`main.c`

```c
void nop(void) { __asm__ volatile ("nop"); } /*@V*/
```

### Violation (must be reported): computed goto and label address

`main.c`

```c
void jump(void)
{
    void *target = &&there; /*@V*/
    goto *target; /*@V*/
there:
    return;
}
```

### Violation (must be reported): case range

`main.c`

```c
int f(int v)
{
    switch (v)
    {
        case 1 ... 3: /*@V*/
            return 1;
        default:
            return 0;
    }
}
```

### Violation (must be reported): typeof (the reserved spelling works in strict C99)

`main.c`

```c
int a;
__typeof__(a) b; /*@V*/
```

### Violation (must be reported): typeof (the plain spelling is a keyword only in GNU mode)

`main.c`

```c
int a;
typeof(a) b; /*@V*/
```

### Violation (must be reported): omitted middle operand of the conditional operator

`main.c`

```c
int f(int x) { return x ?: 5; } /*@V*/
```

### Violation (must be reported): explicit attribute

`main.c`

```c
int unused_value __attribute__((unused)); /*@V*/
```

### Compliant (must stay silent): a function-like macro is not an extension

`main.c`

```c
#define SQUARE(x) ((x) * (x))
int f(int v) { return SQUARE(v); }
```

### Violation (must be reported): attribute on a variable

`main.c`

```c
int x __attribute__((unused)); /*@V*/
```

### Violation (must be reported): attribute on a function declaration

`main.c`

```c
void f(void) __attribute__((noreturn)); /*@V*/
```

### Violation (must be reported): binary literal

`main.c`

```c
int b = 0b101; /*@V*/
```

### Violation (must be reported): zero-length array

`main.c`

```c
struct s { int n; int d[0]; /*@V*/ };
```

### Compliant (must stay silent): flexible array member is C99

`main.c`

```c
struct s { int n; int d[]; };
```

### Compliant (must stay silent): restrict, inline, compound literal and long long are C99

`main.c`

```c
static inline int f(int *restrict p) { long long w = 1LL; return *p + (int)w + (int){3}; }
```

### Compliant (must stay silent): designated initializers are C99

`main.c`

```c
int a[4] = { [2] = 1 };
struct p { int x; int y; } v = { .y = 2 };
```

### Compliant (must stay silent): a comment mentioning __attribute__ and asm

`main.c`

```c
/* __attribute__((x)) asm("nop") */
const char *s = "__attribute__ typeof asm";
```

### Violation (must be reported): statement expression inside a macro use

`main.c`

```c
#define SQ(x) ({ int t = (x); t * t; })
int f(int a) { return SQ(a); } /*@V*/
```

### Violation (must be reported): inline assembly with volatile

`main.c`

```c
void f(void)
{
    __asm__ volatile ("nop"); /*@V*/
}
```

### Violation (must be reported): case range in a switch with several labels

`main.c`

```c
int f(int v)
{
    switch (v)
    {
        case 1: return 1;
        case 2 ... 5: return 2; /*@V*/
        default: return 0;
    }
}
```

