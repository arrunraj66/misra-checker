# Rule 11.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports an explicit cast between pointers to different object types (void pointers are excluded).

Detection contract: Report an explicit cast between pointers to differing object types (void pointers excluded).

## Known limits

Casts to and from character-type pointers are reported (strict reading; Coverity and cppcheck do not report them, so this is undecided).

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | same type, qualifier changes handled elsewhere | pass |
| Compliant (must stay silent) | conversion through void pointer is Rule 11.5 | pass |
| Violation (must be reported) | int pointer to float pointer | pass |
| Violation (must be reported) | struct pointer to int pointer | pass |
| Violation (must be reported) | int pointer to char pointer (strict reading) | pass |
| Compliant (must stay silent) | cast through a typedef of the same type | pass |
| Violation (must be reported) | pointer to double cast to pointer to long | pass |
| Compliant (must stay silent) | a cast that only adds a qualifier is Rule 11.8 compliant and not a type change | pass |
| Compliant (must stay silent) | array decays to a pointer of its own element type | pass |

## Case sources

### Compliant (must stay silent): same type, qualifier changes handled elsewhere

`main.c`

```c
int *f(int *p) { return (int *)p; }
```

### Compliant (must stay silent): conversion through void pointer is Rule 11.5

`main.c`

```c
void *g(int *p) { return (void *)p; }
```

### Violation (must be reported): int pointer to float pointer

`main.c`

```c
float *f(int *p) { return (float *)p; } /*@V*/
```

### Violation (must be reported): struct pointer to int pointer

`main.c`

```c
struct s { int a; };
int *g(struct s *p) { return (int *)p; } /*@V*/
```

### Violation (must be reported): int pointer to char pointer (strict reading)

`main.c`

```c
char *h(int *p) { return (char *)p; } /*@V*/
```

### Compliant (must stay silent): cast through a typedef of the same type

`main.c`

```c
typedef int word_t;
word_t *f(int *p) { return (word_t *)p; }
```

### Violation (must be reported): pointer to double cast to pointer to long

`main.c`

```c
long *f(double *p) { return (long *)p; } /*@V*/
```

### Compliant (must stay silent): a cast that only adds a qualifier is Rule 11.8 compliant and not a type change

`main.c`

```c
const int *f(int *p) { return (const int *)p; }
```

### Compliant (must stay silent): array decays to a pointer of its own element type

`main.c`

```c
char buf[4];
char *f(void) { return (char *)buf; }
```

