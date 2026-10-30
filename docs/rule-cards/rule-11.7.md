# Rule 11.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports a cast between a pointer and a floating type.

Detection contract: Report a cast between a pointer and a floating type. Clang rejects such casts, so the check is reached only through extensions.

## Known limits

Compilers reject such casts, so the check is reached only through compiler extensions. No violating example can be compiled in standard mode.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | pointer and floating objects without a cast between them | pass |
| Compliant (must stay silent) | casts that are not between pointers and floating types | pass |
| Compliant (must stay silent) | pointer to integer to float is two separate steps | pass |
| Compliant (must stay silent) | float to integer and nothing else | pass |

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

