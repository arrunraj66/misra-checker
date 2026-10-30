# Rule 21.9

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to bsearch or qsort (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: bsearch or qsort called (matched by callee name).

## Known limits

Only the two functions of stdlib.h are covered.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a hand written search | pass |
| Violation (must be reported) | qsort and bsearch | pass |
| Violation (must be reported) | qsort with a comparator | pass |

## Case sources

### Compliant (must stay silent): a hand written search

`main.c`

```c
int find(const int *a, int n, int key) { int i; for (i = 0; i < n; i++) { if (a[i] == key) { return i; } } return -1; }
```

### Violation (must be reported): qsort and bsearch

`main.c`

```c
#include <stdlib.h>
int cmp(const void *a, const void *b) { (void)a; (void)b; return 0; }
void f(int *a)
{
    qsort(a, 4, sizeof(int), cmp); /*@V*/
    (void)bsearch(a, a, 4, sizeof(int), cmp); /*@V*/
}
```

### Violation (must be reported): qsort with a comparator

`main.c`

```c
#include <stdlib.h>
int cmp(const void *a, const void *b);
void f(int *t, unsigned long n) { qsort(t, n, sizeof(int), cmp); } /*@V*/
```

