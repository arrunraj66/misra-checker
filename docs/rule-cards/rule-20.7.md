# Rule 20.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a function-like macro that uses a parameter in the replacement list without parentheses around it or adjacent commas (operands of # and ## and member names are excluded).

Detection contract: Report a function-like macro using a parameter in the replacement list without adjacent parentheses or commas (operands of # and ## and member names are excluded).

## Known limits

Whether an expression is safely delimited is judged textually.

## Tested cases

13 of 13 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | parenthesized parameters | pass |
| Compliant (must stay silent) | operands of # and member selection are excluded | pass |
| Compliant (must stay silent) | a parameter between commas | pass |
| Violation (must be reported) | an unparenthesized parameter in an expression | pass |
| Violation (must be reported) | a parameter next to an operator | pass |
| Compliant (must stay silent) | parameter used as a call argument and as a cast operand | pass |
| Violation (must be reported) | parameter on one side of an addition | pass |
| Compliant (must stay silent) | parameter in an array subscript | pass |
| Compliant (must stay silent) | parameter used with the arrow operator in parentheses | pass |
| Violation (must be reported) | parameter in a shift | pass |
| Compliant (must stay silent) | a conditional with every parameter parenthesized | pass |
| Compliant (must stay silent) | a parameter inside an index with parentheses | pass |
| Violation (must be reported) | a parameter in a comparison without parentheses | pass |

## Case sources

### Compliant (must stay silent): parenthesized parameters

`main.c`

```c
#define SQUARE(x) ((x) * (x))
int a = SQUARE(3);
```

### Compliant (must stay silent): operands of # and member selection are excluded

`main.c`

```c
#define STR(x) #x
#define MEMBER(s, m) ((s).m)
struct p { int x; } pt;
int b = MEMBER(pt, x);
```

### Compliant (must stay silent): a parameter between commas

`main.c`

```c
#define CALL(a, b) fn(a, b)
int fn(int, int);
int c = CALL(1, 2);
```

### Violation (must be reported): an unparenthesized parameter in an expression

`main.c`

```c
#define SQUARE(x) x * x /*@V*/
int a = SQUARE(3);
```

### Violation (must be reported): a parameter next to an operator

`main.c`

```c
#define ADD_ONE(x) x + 1 /*@V*/
int b = ADD_ONE(3);
```

### Compliant (must stay silent): parameter used as a call argument and as a cast operand

`main.c`

```c
#define CALL(f, x) f(x)
#define CAST(t, x) ((t)(x))
int g(int a);
int h(void) { return CALL(g, 1); }
long s2(void) { return CAST(long, 2); }
```

### Violation (must be reported): parameter on one side of an addition

`main.c`

```c
#define ADD(x, y) ((x) + y) /*@V*/
int a = ADD(1, 2);
```

### Compliant (must stay silent): parameter in an array subscript

`main.c`

```c
#define AT(a, i) ((a)[i])
int t[3];
int v(void) { return AT(t, 1); }
```

### Compliant (must stay silent): parameter used with the arrow operator in parentheses

`main.c`

```c
struct s { int f; };
#define FIELD(p) ((p)->f)
struct s o;
int v(void) { return FIELD(&o); }
```

### Violation (must be reported): parameter in a shift

`main.c`

```c
#define SHL(x, n) ((x) << n) /*@V*/
int v = SHL(1, 2);
```

### Compliant (must stay silent): a conditional with every parameter parenthesized

`main.c`

```c
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
int m = MAX(1, 2);
```

### Compliant (must stay silent): a parameter inside an index with parentheses

`main.c`

```c
int arr[4];
#define ELEM(i) arr[(i)]
int e = ELEM(2);
```

### Violation (must be reported): a parameter in a comparison without parentheses

`main.c`

```c
#define BIGGER(a, b) a > b /*@V*/
int c = BIGGER(2, 1);
```

