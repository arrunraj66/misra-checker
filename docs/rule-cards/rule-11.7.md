# Rule 11.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports a cast between a pointer and a floating type.

Detection contract: Report a cast between a pointer and a floating type. Clang rejects such casts, so the check is reached only through extensions.

## Known limits

Compilers reject such casts, so the check is reached only through compiler extensions. No violating example can be compiled in standard mode.

## Tested cases

2 of 2 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | pointer and floating objects without a cast between them | pass |
| Compliant (must stay silent) | casts that are not between pointers and floating types | pass |

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

