# Rule 20.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a function-like macro that uses a parameter in the replacement list without parentheses around it or adjacent commas (operands of # and ## and member names are excluded).

Detection contract: Report a function-like macro using a parameter in the replacement list without adjacent parentheses or commas (operands of # and ## and member names are excluded).

## Known limits

Whether an expression is safely delimited is judged textually.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | parenthesized parameters | pass |
| Compliant (must stay silent) | operands of # and member selection are excluded | pass |
| Compliant (must stay silent) | a parameter between commas | pass |
| Violation (must be reported) | an unparenthesized parameter in an expression | pass |
| Violation (must be reported) | a parameter next to an operator | pass |
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

