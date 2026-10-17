# Rule 18.5

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointers and arrays

## What the detector reports

Reports a declaration whose type has more than two levels of pointer nesting.

Detection contract: Report a declaration whose type has more than two levels of pointer nesting.

## Known limits

The levels are counted in the declaration as written; a typedef name counts as one level, so a typedef of a two-level pointer used through one more pointer is not reported (each declaration has at most two levels).

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | up to two levels | pass |
| Violation (must be reported) | three levels | pass |
| Compliant (must stay silent) | a typedef counts as one level in the declaration that uses it | pass |
| Violation (must be reported) | three levels written in a parameter | pass |

## Case sources

### Compliant (must stay silent): up to two levels

`main.c`

```c
int *a;
int **b;
char *argv[4];
```

### Violation (must be reported): three levels

`main.c`

```c
int ***c; /*@V*/
```

### Compliant (must stay silent): a typedef counts as one level in the declaration that uses it

`main.c`

```c
typedef int **pp_t;
void f(pp_t *q);
```

### Violation (must be reported): three levels written in a parameter

`main.c`

```c
void g(int ***r); /*@V*/
```

