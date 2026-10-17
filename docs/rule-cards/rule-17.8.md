# Rule 17.8

**Category:** Advisory · **Decidability:** Undecidable · **Scope:** System · **Topic:** Functions

## What the detector reports

Reports assignment, compound assignment, increment or decrement applied directly to a function parameter.

Detection contract: Report assignment, compound assignment, increment or decrement applied directly to a function parameter.

## Known limits

Modifying the object a pointer parameter points to is allowed; only the parameter itself is examined.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | the pointee is modified | pass |
| Compliant (must stay silent) | a local copy is modified | pass |
| Violation (must be reported) | assignment to a parameter | pass |
| Violation (must be reported) | increment of a parameter | pass |
| Violation (must be reported) | compound assignment to a pointer parameter | pass |

## Case sources

### Compliant (must stay silent): the pointee is modified

`main.c`

```c
void set(int *p) { *p = 1; (*p)++; }
```

### Compliant (must stay silent): a local copy is modified

`main.c`

```c
int f(int a) { int local = a; local++; return local; }
```

### Violation (must be reported): assignment to a parameter

`main.c`

```c
int f(int a)
{
    a = 1; /*@V*/
    return a;
}
```

### Violation (must be reported): increment of a parameter

`main.c`

```c
int g(int a)
{
    a++; /*@V*/
    return a;
}
```

### Violation (must be reported): compound assignment to a pointer parameter

`main.c`

```c
void h(char *p)
{
    p += 1; /*@V*/
    (void)p;
}
```

