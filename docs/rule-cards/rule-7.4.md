# Rule 7.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Literals and constants

## What the detector reports

Reports a string literal that converts to a pointer to non-const char outside subscript and dereference contexts.

Detection contract: Report a string literal that converts to a pointer to non-const char outside subscript and dereference contexts.

## Known limits

Only conversions the compiler shows as a pointer decay are examined; a literal that initializes a char array copies the characters and is accepted.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | pointer to const char | pass |
| Compliant (must stay silent) | the literal initializes an array | pass |
| Compliant (must stay silent) | subscript, dereference and sizeof | pass |
| Violation (must be reported) | pointer to non-const char initialized with a literal | pass |
| Violation (must be reported) | assignment of a literal to a char pointer | pass |
| Violation (must be reported) | literal passed to a function that takes char * | pass |
| Compliant (must stay silent) | literal passed to a function that takes const char * | pass |

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

