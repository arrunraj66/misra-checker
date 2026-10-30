# Rule 17.6

**Category:** Mandatory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Functions

## What the detector reports

Reports an array parameter declared with the static keyword inside the brackets.

Detection contract: Report an array parameter declared with the static keyword inside the brackets.

## Known limits

Only the bracketed form is examined.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary array parameter | pass |
| Violation (must be reported) | static inside the brackets | pass |
| Violation (must be reported) | in a definition | pass |
| Compliant (must stay silent) | const inside the brackets | pass |
| Violation (must be reported) | static with a size in a prototype | pass |
| Compliant (must stay silent) | an ordinary array parameter with a size | pass |
| Violation (must be reported) | a definition with a static size and several parameters | pass |

## Case sources

### Compliant (must stay silent): ordinary array parameter

`main.c`

```c
void f(int a[4]);
void g(int b[]);
```

### Violation (must be reported): static inside the brackets

`main.c`

```c
void f(int a[static 4]); /*@V*/
```

### Violation (must be reported): in a definition

`main.c`

```c
int sum(int a[static 2]) { return a[0] + a[1]; } /*@V*/
```

### Compliant (must stay silent): const inside the brackets

`main.c`

```c
void f(int a[const 4]);
```

### Violation (must be reported): static with a size in a prototype

`main.c`

```c
void f(char s[static 8]); /*@V*/
```

### Compliant (must stay silent): an ordinary array parameter with a size

`main.c`

```c
void f(int a[4]);
```

### Violation (must be reported): a definition with a static size and several parameters

`main.c`

```c
int f(int n, int a[static 2])  /*@V*/
{
    return n + a[0];
}
```

