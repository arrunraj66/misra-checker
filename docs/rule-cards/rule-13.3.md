# Rule 13.3

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Side effects and evaluation order

## What the detector reports

Reports an increment or decrement that is not a whole expression statement and shares its full expression with another side effect.

Detection contract: Report an increment or decrement that is not a whole expression statement and shares its full expression with another side effect.

## Known limits

A bare statement i++; is accepted. A single increment used for its value without another side effect in the expression is accepted.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | increments as statements | pass |
| Compliant (must stay silent) | an increment in a for statement clause | pass |
| Violation (must be reported) | increment combined with an assignment | pass |
| Violation (must be reported) | increment inside an index expression | pass |
| Violation (must be reported) | decrement combined with a call | pass |
| Compliant (must stay silent) | an increment alone in a condition comparison | pass |
| Violation (must be reported) | an increment as a call argument | pass |
| Violation (must be reported) | a post-increment in an indexed store | pass |
| Compliant (must stay silent) | pre-increment as a statement | pass |

## Case sources

### Compliant (must stay silent): increments as statements

`main.c`

```c
void f(int *p) { int i = 0; i++; --i; (*p)++; }
```

### Compliant (must stay silent): an increment in a for statement clause

`main.c`

```c
int g(int n) { int s = 0; int i; for (i = 0; i < n; i++) { s += i; } return s; }
```

### Violation (must be reported): increment combined with an assignment

`main.c`

```c
int f(int i)
{
    int x;
    x = i++; /*@V*/
    return x + i;
}
```

### Violation (must be reported): increment inside an index expression

`main.c`

```c
int g(int *a, int i)
{
    a[i++] = 0; /*@V*/
    return i;
}
```

### Violation (must be reported): decrement combined with a call

`main.c`

```c
void use(int);
void h(int i) { use(i--); } /*@V*/
```

### Compliant (must stay silent): an increment alone in a condition comparison

`main.c`

```c
int f(int i) { return i++ > 1; }
```

### Violation (must be reported): an increment as a call argument

`main.c`

```c
int g(int a);
int f(int i) { return g(i++); } /*@V*/
```

### Violation (must be reported): a post-increment in an indexed store

`main.c`

```c
void f(int *t, int i) { t[i++] = 0; } /*@V*/
```

### Compliant (must stay silent): pre-increment as a statement

`main.c`

```c
void f(int i) { ++i; (void)i; }
```

