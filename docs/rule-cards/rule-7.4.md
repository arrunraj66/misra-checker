# Rule 7.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Literals and constants

## What the detector reports

Reports a string literal that converts to a pointer to non-const char outside subscript and dereference contexts.

Detection contract: Report a string literal that converts to a pointer to non-const char outside subscript and dereference contexts.

## Known limits

Only conversions the compiler shows as a pointer decay are examined; a literal that initializes a char array copies the characters and is accepted.

## Tested cases

16 of 16 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | pointer to const char | pass |
| Compliant (must stay silent) | the literal initializes an array | pass |
| Compliant (must stay silent) | subscript, dereference and sizeof | pass |
| Violation (must be reported) | pointer to non-const char initialized with a literal | pass |
| Violation (must be reported) | assignment of a literal to a char pointer | pass |
| Violation (must be reported) | literal passed to a function that takes char * | pass |
| Compliant (must stay silent) | literal passed to a function that takes const char * | pass |
| Violation (must be reported) | literal returned from a function that returns char * | pass |
| Violation (must be reported) | array of char pointers initialized with literals | pass |
| Compliant (must stay silent) | array of const char pointers | pass |
| Compliant (must stay silent) | literal passed to printf-like const char * parameter | pass |
| Violation (must be reported) | conditional of two literals assigned to char * | pass |
| Compliant (must stay silent) | conditional of two literals assigned to const char * | pass |
| Compliant (must stay silent) | comparison of a literal pointer with another pointer | pass |
| Compliant (must stay silent) | char array initialised from a literal | pass |
| Compliant (must stay silent) | literal as an argument to a char-array safe function taking const | pass |

## Case sources

### Compliant (must stay silent): pointer to const char

`main.c`

```c
const char *p = "text";
```

### Compliant (must stay silent): the literal initializes an array

`main.c`

```c
char buffer[] = "text";
```

### Compliant (must stay silent): subscript, dereference and sizeof

`main.c`

```c
char first(void) { return "abc"[1]; }
char second(void) { return *"abc"; }
unsigned long n = sizeof("abc");
```

### Violation (must be reported): pointer to non-const char initialized with a literal

`main.c`

```c
char *p = "text"; /*@V*/
```

### Violation (must be reported): assignment of a literal to a char pointer

`main.c`

```c
void f(void)
{
    char *q;
    q = "other"; /*@V*/
    (void)q;
}
```

### Violation (must be reported): literal passed to a function that takes char *

`main.c`

```c
void take(char *s);
void g(void) { take("literal"); } /*@V*/
```

### Compliant (must stay silent): literal passed to a function that takes const char *

`main.c`

```c
void take_const(const char *s);
void h(void) { take_const("literal"); }
```

### Violation (must be reported): literal returned from a function that returns char *

`main.c`

```c
char *f(void) { return "x"; } /*@V*/
```

### Violation (must be reported): array of char pointers initialized with literals

`main.c`

```c
char *names[2] = { "a", "b" }; /*@V*/
```

### Compliant (must stay silent): array of const char pointers

`main.c`

```c
const char *names[2] = { "a", "b" };
```

### Compliant (must stay silent): literal passed to printf-like const char * parameter

`main.c`

```c
int p(const char *fmt, ...);
void f(void) { (void)p("x"); }
```

### Violation (must be reported): conditional of two literals assigned to char *

`main.c`

```c
char *f(int c)
{
    char *p = c ? "a" : "b"; /*@V*/
    return p;
}
```

### Compliant (must stay silent): conditional of two literals assigned to const char *

`main.c`

```c
const char *f(int c) { return c ? "a" : "b"; }
```

### Compliant (must stay silent): comparison of a literal pointer with another pointer

`main.c`

```c
int f(const char *p) { return p == "x"; }
```

### Compliant (must stay silent): char array initialised from a literal

`main.c`

```c
char buf[4] = "abc";
char big[] = "abcdef";
```

### Compliant (must stay silent): literal as an argument to a char-array safe function taking const

`main.c`

```c
unsigned long len(const char *s);
unsigned long n = 0;
void f(void) { n = len("abc"); }
```

