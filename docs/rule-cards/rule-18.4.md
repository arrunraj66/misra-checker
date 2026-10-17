# Rule 18.4

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointers and arrays

## What the detector reports

Reports +, -, += and -= applied to an operand of pointer type.

Detection contract: Report +, -, += or -= with a pointer-typed operand.

## Known limits

Subscripting, dereference and increment or decrement are not reported.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | subscripts and dereference | pass |
| Compliant (must stay silent) | increment (not one of the listed operators) | pass |
| Violation (must be reported) | pointer plus integer | pass |
| Violation (must be reported) | pointer minus integer | pass |
| Violation (must be reported) | compound assignments on a pointer | pass |
| Violation (must be reported) | an array name decays to a pointer | pass |
| Violation (must be reported) | the difference of two pointers | pass |
| Compliant (must stay silent) | subscript instead of arithmetic | pass |

## Case sources

### Compliant (must stay silent): subscripts and dereference

`main.c`

```c
int f(int *p) { return p[1] + *p; }
```

### Compliant (must stay silent): increment (not one of the listed operators)

`main.c`

```c
int g(int *p) { p++; return *p; }
```

### Violation (must be reported): pointer plus integer

`main.c`

```c
int *f(int *p) { return p + 1; } /*@V*/
```

### Violation (must be reported): pointer minus integer

`main.c`

```c
int *g(int *p) { return p - 1; } /*@V*/
```

### Violation (must be reported): compound assignments on a pointer

`main.c`

```c
void h(int *p)
{
    p += 2; /*@V*/
    p -= 1; /*@V*/
    (void)p;
}
```

### Violation (must be reported): an array name decays to a pointer

`main.c`

```c
int a[4];
int *k(void) { return a + 1; } /*@V*/
```

### Violation (must be reported): the difference of two pointers

`main.c`

```c
long d(int *p, int *q) { return p - q; } /*@V*/
```

### Compliant (must stay silent): subscript instead of arithmetic

`main.c`

```c
int f(int *p, int i) { return p[i]; }
```

