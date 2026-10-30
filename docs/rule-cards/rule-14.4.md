# Rule 14.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Control statement expressions

## What the detector reports

Reports an if, loop or conditional-operator condition that is not essentially Boolean (a relational, equality, logical or negation expression, or a _Bool).

Detection contract: Report an if, loop or conditional-operator condition that is not essentially Boolean (relational, equality, logical, negation, or _Bool).

## Known limits

Only controlling expressions are examined; operands of && and || are Rule 10.1's concern.

## Tested cases

14 of 14 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | essentially Boolean conditions | pass |
| Compliant (must stay silent) | negation and logical operators | pass |
| Violation (must be reported) | an integer as a condition | pass |
| Violation (must be reported) | a pointer as a condition | pass |
| Violation (must be reported) | an integer as the condition of the conditional operator | pass |
| Violation (must be reported) | while(1) is not essentially Boolean | pass |
| Compliant (must stay silent) | for(;;) | pass |
| Compliant (must stay silent) | Boolean variable and double negation | pass |
| Violation (must be reported) | integer variable in a for condition | pass |
| Violation (must be reported) | floating variable as a condition | pass |
| Compliant (must stay silent) | a masked comparison and a pointer comparison | pass |
| Compliant (must stay silent) | a Boolean valued macro expression | pass |
| Violation (must be reported) | a bit test without comparison | pass |
| Violation (must be reported) | a function result used as a condition | pass |

## Case sources

### Compliant (must stay silent): essentially Boolean conditions

`main.c`

```c
int f(int x, _Bool flag) { if (x > 0) { return 1; } while (x != 0) { x--; } return flag ? 2 : 3; }
```

### Compliant (must stay silent): negation and logical operators

`main.c`

```c
int g(int a, int b) { if (!a) { return 1; } if (a && b) { return 2; } return 0; }
```

### Violation (must be reported): an integer as a condition

`main.c`

```c
int f(int x)
{
    if (x) { return 1; } /*@V*/
    return 0;
}
```

### Violation (must be reported): a pointer as a condition

`main.c`

```c
int g(int *p)
{
    while (p) { p = 0; } /*@V*/
    return 0;
}
```

### Violation (must be reported): an integer as the condition of the conditional operator

`main.c`

```c
int h(int x) { return x ? 1 : 2; } /*@V*/
```

### Violation (must be reported): while(1) is not essentially Boolean

`main.c`

```c
void f(void) { while (1) { } } /*@V*/
```

### Compliant (must stay silent): for(;;)

`main.c`

```c
void f(void) { for (;;) { } }
```

### Compliant (must stay silent): Boolean variable and double negation

`main.c`

```c
int f(_Bool b, int x) { if (b) { return 1; } if (!!x) { return 2; } return 0; }
```

### Violation (must be reported): integer variable in a for condition

`main.c`

```c
void f(int n) { for (; n; n--) { } } /*@V*/
```

### Violation (must be reported): floating variable as a condition

`main.c`

```c
int f(float x) { if (x) { return 1; } return 0; } /*@V*/
```

### Compliant (must stay silent): a masked comparison and a pointer comparison

`main.c`

```c
int f(int x, int *p) { if ((x & 4) != 0) { return 1; } if (p != 0) { return 2; } return 0; }
```

### Compliant (must stay silent): a Boolean valued macro expression

`main.c`

```c
#define IS_SET(x, m) (((x) & (m)) != 0)
int f(int x) { if (IS_SET(x, 4)) { return 1; } return 0; }
```

### Violation (must be reported): a bit test without comparison

`main.c`

```c
int f(int x)
{
    if (x & 4) { return 1; } /*@V*/
    return 0;
}
```

### Violation (must be reported): a function result used as a condition

`main.c`

```c
int check(void);
void f(void)
{
    while (check()) { } /*@V*/
}
```

