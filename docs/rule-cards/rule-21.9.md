# Rule 21.9

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to bsearch or qsort (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: bsearch or qsort called (matched by callee name).

## Known limits

Only the two functions of stdlib.h are covered.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | a hand written search | pass |
| Violation (must be reported) | qsort and bsearch | pass |
| Violation (must be reported) | qsort with a comparator | pass |
| Violation (must be reported) | bsearch | pass |
| Compliant (must stay silent) | a local function called search | pass |

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

### Violation (must be reported): bsearch

`main.c`

```c
#include <stdlib.h>
int cmp(const void *a, const void *b);
void *f(const void *k, const void *t, unsigned long n)
{
    return bsearch(k, t, n, 4, cmp); /*@V*/
}
```

### Compliant (must stay silent): a local function called search

`main.c`

```c
int search(const int *t, int n, int k);
int f(const int *t, int n) { return search(t, n, 3); }
```

