# Rule 21.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a call to a standard input/output function (matched by the callee's name).

Detection contract: Report a call to one of the listed functions: standard input/output function called (matched by callee name).

## Known limits

Only the listed stdio and wide-character I/O functions are covered.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no I/O | pass |
| Compliant (must stay silent) | a user function with a similar name | pass |
| Violation (must be reported) | printf and puts | pass |
| Violation (must be reported) | file functions | pass |
| Violation (must be reported) | fopen and fclose | pass |
| Compliant (must stay silent) | a structure member named printf | pass |
| Violation (must be reported) | puts and putchar | pass |
| Violation (must be reported) | formatted input and output | pass |
| Violation (must be reported) | character and block I/O | pass |

## Case sources

### Compliant (must stay silent): no I/O

`main.c`

```c
int f(int x) { return x; }
```

### Compliant (must stay silent): a user function with a similar name

`main.c`

```c
int print_value(int v);
void g(void) { (void)print_value(1); }
```

### Violation (must be reported): printf and puts

`main.c`

```c
#include <stdio.h>
void f(void)
{
    (void)printf("x"); /*@V*/
    (void)puts("y"); /*@V*/
}
```

### Violation (must be reported): file functions

`main.c`

```c
#include <stdio.h>
void g(void)
{
    FILE *f = fopen("a", "r"); /*@V*/
    (void)fclose(f); /*@V*/
}
```

### Violation (must be reported): fopen and fclose

`main.c`

```c
#include <stdio.h>
void f(void)
{
    FILE *p = fopen("a", "r"); /*@V*/
    if (p != NULL) { (void)fclose(p); } /*@V*/
}
```

### Compliant (must stay silent): a structure member named printf

`main.c`

```c
struct s { int printf; };
int f(struct s *p) { return p->printf; }
```

### Violation (must be reported): puts and putchar

`main.c`

```c
#include <stdio.h>
void f(void)
{
    (void)puts("x"); /*@V*/
    (void)putchar('a'); /*@V*/
}
```

### Violation (must be reported): formatted input and output

`main.c`

```c
#include <stdio.h>
void f(char *buf)
{
    (void)sprintf(buf, "%d", 1); /*@V*/
    (void)snprintf(buf, 8, "%d", 1); /*@V*/
    (void)scanf("%d", (int *)0); /*@V*/
}
```

### Violation (must be reported): character and block I/O

`main.c`

```c
#include <stdio.h>
void g(char *buf, FILE *s)
{
    (void)getchar(); /*@V*/
    (void)fgets(buf, 4, s); /*@V*/
    (void)fread(buf, 1, 4, s); /*@V*/
}
```

