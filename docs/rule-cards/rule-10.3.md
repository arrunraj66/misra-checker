# Rule 10.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Reports an assignment or initialization of a non-constant value to an object of a different essential type category or of a narrower width.

Detection contract: Report assignment or initialization of a non-constant value to an object of a different essential type category or narrower width.

## Known limits

Constants are exempt. Only assignments and initializers are examined; function arguments and returns are Rule 10.3's neighbours but are not covered here.

## Tested cases

19 of 19 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | same category, same or wider width | pass |
| Compliant (must stay silent) | constants may initialize narrower objects | pass |
| Compliant (must stay silent) | a conditional keeps the type of its operands | pass |
| Violation (must be reported) | narrowing assignment | pass |
| Violation (must be reported) | different category (int to float) | pass |
| Violation (must be reported) | integer to Boolean | pass |
| Violation (must be reported) | wider to narrower within the same category | pass |
| Violation (must be reported) | wider integer passed to a narrower parameter | pass |
| Violation (must be reported) | wider value returned from a narrower function | pass |
| Violation (must be reported) | integer assigned to an enumeration | pass |
| Compliant (must stay silent) | same type returned | pass |
| Compliant (must stay silent) | unsigned char passed as an unsigned int (wider, same category) | pass |
| Compliant (must stay silent) | float assigned to double | pass |
| Violation (must be reported) | double assigned to float | pass |
| Compliant (must stay silent) | widening within a category and constants | pass |
| Violation (must be reported) | a character assigned to an int (a different essential type category) | pass |
| Violation (must be reported) | a signed value assigned to an unsigned object | pass |
| Violation (must be reported) | an int assigned to a char | pass |
| Violation (must be reported) | a floating value assigned to an integer | pass |

## Case sources

### Compliant (must stay silent): same category, same or wider width

`main.c`

```c
int f(int a) { long wide = a; int same = a; return (int)wide + same; }
```

### Compliant (must stay silent): constants may initialize narrower objects

`main.c`

```c
unsigned char u = 3;
```

### Compliant (must stay silent): a conditional keeps the type of its operands

`main.c`

```c
int g(int c, int a, int b) { int r = c ? a : b; return r; }
```

### Violation (must be reported): narrowing assignment

`main.c`

```c
void f(int i)
{
    unsigned char u;
    u = i; /*@V*/
    (void)u;
}
```

### Violation (must be reported): different category (int to float)

`main.c`

```c
void g(int i)
{
    float f = i; /*@V*/
    (void)f;
}
```

### Violation (must be reported): integer to Boolean

`main.c`

```c
void h(int i)
{
    _Bool b = i; /*@V*/
    (void)b;
}
```

### Violation (must be reported): wider to narrower within the same category

`main.c`

```c
void k(long l)
{
    int n = l; /*@V*/
    (void)n;
}
```

### Violation (must be reported): wider integer passed to a narrower parameter

`main.c`

```c
void g(unsigned char c);
void f(int i) { g(i); } /*@V*/
```

### Violation (must be reported): wider value returned from a narrower function

`main.c`

```c
unsigned char f(int i) { return i; } /*@V*/
```

### Violation (must be reported): integer assigned to an enumeration

`main.c`

```c
enum e { A, B };
void f(int i) { enum e v = i; (void)v; } /*@V*/
```

### Compliant (must stay silent): same type returned

`main.c`

```c
int f(int i) { return i; }
```

### Compliant (must stay silent): unsigned char passed as an unsigned int (wider, same category)

`main.c`

```c
void g(unsigned int c);
void f(unsigned char c) { g(c); }
```

### Compliant (must stay silent): float assigned to double

`main.c`

```c
void f(float x) { double d = x; (void)d; }
```

### Violation (must be reported): double assigned to float

`main.c`

```c
void f(double x) { float d = x; (void)d; } /*@V*/
```

### Compliant (must stay silent): widening within a category and constants

`main.c`

```c
int f(char c, int i)
{
    long l = i;
    char k = 'a';
    unsigned char u = 200;
    (void)l; (void)c; (void)k; (void)u;
    return 0;
}
```

### Violation (must be reported): a character assigned to an int (a different essential type category)

`main.c`

```c
void g(char c)
{
    int j = c; /*@V*/
    (void)j;
}
```

### Violation (must be reported): a signed value assigned to an unsigned object

`main.c`

```c
void f(int i)
{
    unsigned int u = i; /*@V*/
    (void)u;
}
```

### Violation (must be reported): an int assigned to a char

`main.c`

```c
void g(int i)
{
    char c = i; /*@V*/
    (void)c;
}
```

### Violation (must be reported): a floating value assigned to an integer

`main.c`

```c
void h(double d)
{
    int n = d; /*@V*/
    (void)n;
}
```

