# Rule 12.3

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Expressions

## What the detector reports

Reports every use of the comma operator.

Detection contract: Report every use of the comma operator.

## Known limits

Commas that separate arguments, declarators and initializers are not the comma operator.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | commas in calls, declarations and initializers | pass |
| Violation (must be reported) | comma operator in a for statement | pass |
| Violation (must be reported) | comma operator in an expression | pass |
| Violation (must be reported) | comma operator in a macro | pass |
| Violation (must be reported) | two comma operators in one for header | pass |
| Compliant (must stay silent) | a declaration list | pass |
| Compliant (must stay silent) | commas in a function-like macro argument list | pass |
| Compliant (must stay silent) | commas in an enumeration and in a struct initializer | pass |
| Violation (must be reported) | the comma operator inside a condition | pass |

## Case sources

### Compliant (must stay silent): commas in calls, declarations and initializers

`main.c`

```c
int add(int a, int b);
int a = 1, b = 2;
int arr[3] = { 1, 2, 3 };
int s = 0;
void f(void) { s = add(a, b); }
```

### Violation (must be reported): comma operator in a for statement

`main.c`

```c
void f(void)
{
    int i, j;
    for (i = 0, j = 10; i < j; i++) { j--; } /*@V*/
}
```

### Violation (must be reported): comma operator in an expression

`main.c`

```c
int g(int a, int b) { return (a++, b); } /*@V*/
```

### Violation (must be reported): comma operator in a macro

`main.c`

```c
#define LAST(a, b) ((void)(a), (b))
int f(int x, int y) { return LAST(x, y); } /*@V*/
```

### Violation (must be reported): two comma operators in one for header

`main.c`

```c
void f(int n)
{
    int i, j;
    for (i = 0, j = n; i < j; i++, j--) { } /*@V*/
}
```

### Compliant (must stay silent): a declaration list

`main.c`

```c
int a, b, c;
```

### Compliant (must stay silent): commas in a function-like macro argument list

`main.c`

```c
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
int m = MAX(1, 2);
```

### Compliant (must stay silent): commas in an enumeration and in a struct initializer

`main.c`

```c
enum e { A, B, C };
struct p { int x; int y; } v = { 1, 2 };
```

### Violation (must be reported): the comma operator inside a condition

`main.c`

```c
int f(int a, int b)
{
    if ((a++, b) > 0) { return 1; } /*@V*/
    return 0;
}
```

