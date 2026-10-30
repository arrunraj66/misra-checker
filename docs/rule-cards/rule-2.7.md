# Rule 2.7

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Unused and unreachable code

## What the detector reports

Reports a named parameter of a function definition that the body never refers to.

Detection contract: Report a named parameter of a function definition that is never referenced in the body.

## Known limits

A parameter referenced only inside a macro that is not expanded counts as unused. Function declarations (without a body) are not examined.

## Tested cases

14 of 14 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | all parameters used | pass |
| Compliant (must stay silent) | a void cast counts as a reference | pass |
| Compliant (must stay silent) | a prototype never uses its parameters | pass |
| Violation (must be reported) | one parameter unused | pass |
| Violation (must be reported) | all parameters unused | pass |
| Violation (must be reported) | unused parameter in a static function | pass |
| Compliant (must stay silent) | parameter used only as an array bound of another parameter | pass |
| Compliant (must stay silent) | parameter passed on to another function | pass |
| Compliant (must stay silent) | parameter used only by taking its address | pass |
| Compliant (must stay silent) | parameter used only in a macro argument that expands to it | pass |
| Violation (must be reported) | parameter used only inside a macro that discards it | pass |
| Compliant (must stay silent) | a parameter used only in sizeof | pass |
| Compliant (must stay silent) | a parameter used only in a condition | pass |
| Violation (must be reported) | a parameter shadowed by a local and never used | pass |

## Case sources

### Compliant (must stay silent): all parameters used

`main.c`

```c
int add(int a, int b) { return a + b; }
```

### Compliant (must stay silent): a void cast counts as a reference

`main.c`

```c
int pick(int a, int b) { (void)b; return a; }
```

### Compliant (must stay silent): a prototype never uses its parameters

`main.c`

```c
int proto(int a, int b);
```

### Violation (must be reported): one parameter unused

`main.c`

```c
int first(int a, int b) { return a; } /*@V*/
```

### Violation (must be reported): all parameters unused

`main.c`

```c
void nothing(int a, char *b) { } /*@V*/
```

### Violation (must be reported): unused parameter in a static function

`main.c`

```c
static int f(int a, int b) /*@V*/
{
    return a;
}
int g(void) { return f(1, 2); }
```

### Compliant (must stay silent): parameter used only as an array bound of another parameter

`main.c`

```c
void f(int n, int a[n])
{
    a[0] = 1;
}
```

### Compliant (must stay silent): parameter passed on to another function

`main.c`

```c
int g(int);
int f(int a) { return g(a); }
```

### Compliant (must stay silent): parameter used only by taking its address

`main.c`

```c
int *f(int a) { static int *p; p = &a; return p; }
```

### Compliant (must stay silent): parameter used only in a macro argument that expands to it

`main.c`

```c
#define TWICE(x) ((x) + (x))
int f(int a) { return TWICE(a); }
```

### Violation (must be reported): parameter used only inside a macro that discards it

`main.c`

```c
#define DROP(x)
int f(int a) /*@V*/
{
    DROP(a)
    return 0;
}
```

### Compliant (must stay silent): a parameter used only in sizeof

`main.c`

```c
unsigned long f(int *p) { return sizeof(*p); }
```

### Compliant (must stay silent): a parameter used only in a condition

`main.c`

```c
int f(int a, int b) { if (b) { return a; } return 0; }
```

### Violation (must be reported): a parameter shadowed by a local and never used

`main.c`

```c
int f(int a) /*@V*/
{
    int a2 = 1;
    return a2;
}
```

