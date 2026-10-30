# Rule 11.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports a cast between a pointer and a floating type.

Detection contract: Report a cast between a pointer and a floating type. Clang rejects such casts, so the check is reached only through extensions.

## Known limits

Compilers reject such casts, so the check is reached only through compiler extensions. No violating example can be compiled in standard mode.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | pointer and floating objects without a cast between them | pass |
| Compliant (must stay silent) | casts that are not between pointers and floating types | pass |
| Compliant (must stay silent) | pointer to integer to float is two separate steps | pass |
| Compliant (must stay silent) | float to integer and nothing else | pass |
| Compliant (must stay silent) | a pointer converted to an integer and back through void | pass |
| Compliant (must stay silent) | a floating value cast to another floating type | pass |
| Compliant (must stay silent) | a pointer to a floating object dereferenced | pass |
| Compliant (must stay silent) | floating zero is not a null pointer constant | pass |

## Case sources

### Compliant (must stay silent): pointer and floating objects without a cast between them

`main.c`

```c
float f;
int *p;
void g(void) { f = 1.0f; p = 0; }
```

### Compliant (must stay silent): casts that are not between pointers and floating types

`main.c`

```c
double d = (double)1;
long l = (long)d;
```

### Compliant (must stay silent): pointer to integer to float is two separate steps

`main.c`

```c
float f(char *p) { unsigned long n = (unsigned long)p; return (float)n; }
```

### Compliant (must stay silent): float to integer and nothing else

`main.c`

```c
long f(float x) { return (long)x; }
```

### Compliant (must stay silent): a pointer converted to an integer and back through void

`main.c`

```c
void f(int *p)
{
    void *v = p;
    int *q = (int *)v;
    (void)q;
}
```

### Compliant (must stay silent): a floating value cast to another floating type

`main.c`

```c
double f(float x) { return (double)x; }
```

### Compliant (must stay silent): a pointer to a floating object dereferenced

`main.c`

```c
double f(const double *p) { return *p + 1.0; }
```

### Compliant (must stay silent): floating zero is not a null pointer constant

`main.c`

```c
double d = 0.0;
```

