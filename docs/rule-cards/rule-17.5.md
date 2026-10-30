# Rule 17.5

**Category:** Advisory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Functions

## What the detector reports

Reports an argument that is smaller than the stated size of its array parameter, or a null argument for one.

Detection contract: Report an argument smaller than the stated size of its array parameter, or a null argument for one.

## Known limits

Only arguments whose array size is known at the call site are checked.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | argument at least as large as the parameter | pass |
| Compliant (must stay silent) | a parameter of unspecified size | pass |
| Violation (must be reported) | argument smaller than the parameter | pass |
| Violation (must be reported) | null argument for an array parameter | pass |
| Violation (must be reported) | a smaller array passed to a two dimensional parameter | pass |
| Compliant (must stay silent) | a larger array than the parameter needs | pass |
| Compliant (must stay silent) | a pointer argument of unknown size | pass |

## Case sources

### Compliant (must stay silent): argument at least as large as the parameter

`main.c`

```c
void f(int a[4]);
int big[8];
int exact[4];
void g(void) { f(big); f(exact); }
```

### Compliant (must stay silent): a parameter of unspecified size

`main.c`

```c
void h(int a[]);
int small[2];
void k(void) { h(small); }
```

### Violation (must be reported): argument smaller than the parameter

`main.c`

```c
void f(int a[4]);
int small[2];
void g(void) { f(small); } /*@V*/
```

### Violation (must be reported): null argument for an array parameter

`main.c`

```c
void f(int a[4]);
void g(void) { f(0); } /*@V*/
```

### Violation (must be reported): a smaller array passed to a two dimensional parameter

`main.c`

```c
void g(int a[2][4]);
void f(void)
{
    int b[1][4];
    g(b); /*@V*/
}
```

### Compliant (must stay silent): a larger array than the parameter needs

`main.c`

```c
void g(int a[4]);
void f(void)
{
    int b[8];
    g(b);
}
```

### Compliant (must stay silent): a pointer argument of unknown size

`main.c`

```c
void g(int a[4]);
void f(int *p) { g(p); }
```

