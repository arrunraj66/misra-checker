# Rule 8.14

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports any object, parameter or member declared with a restrict-qualified type.

Detection contract: Report any object, parameter or member with a restrict-qualified type.

## Known limits

The qualifier is reported wherever it is written.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no restrict | pass |
| Violation (must be reported) | restrict parameters | pass |
| Violation (must be reported) | restrict pointer object | pass |
| Violation (must be reported) | restrict member in a structure | pass |
| Compliant (must stay silent) | the word restrict inside an identifier | pass |
| Violation (must be reported) | restrict through a typedef | pass |
| Compliant (must stay silent) | no pointer parameter uses the keyword | pass |
| Violation (must be reported) | restrict on a local pointer | pass |

## Case sources

### Compliant (must stay silent): no restrict

`main.c`

```c
void copy(int *dst, const int *src, int n);
```

### Violation (must be reported): restrict parameters

`main.c`

```c
void copy(int *restrict dst, const int *restrict src, int n) { } /*@V*/
```

### Violation (must be reported): restrict pointer object

`main.c`

```c
int *restrict global_ptr; /*@V*/
```

### Violation (must be reported): restrict member in a structure

`main.c`

```c
struct s { int *restrict p; }; /*@V*/
```

### Compliant (must stay silent): the word restrict inside an identifier

`main.c`

```c
int restricted;
int restrict_count;
```

### Violation (must be reported): restrict through a typedef

`main.c`

```c
typedef int *restrict rp_t; /*@V*/
```

### Compliant (must stay silent): no pointer parameter uses the keyword

`main.c`

```c
void copy(int *dst, const int *src, unsigned n);
```

### Violation (must be reported): restrict on a local pointer

`main.c`

```c
void f(void)
{
    int a = 0;
    int *restrict p = &a; /*@V*/
    *p = 1;
}
```

