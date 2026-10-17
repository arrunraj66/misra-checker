# Rule 11.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports an explicit cast between a pointer to void and an integer type.

Detection contract: Report an explicit cast between pointer-to-void and an integer type.

## Known limits

A null pointer constant is exempt.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | null pointer | pass |
| Compliant (must stay silent) | pointers only | pass |
| Violation (must be reported) | integer cast to void pointer | pass |
| Violation (must be reported) | void pointer cast to integer | pass |

## Case sources

### Compliant (must stay silent): null pointer

`main.c`

```c
void *p = (void *)0;
```

### Compliant (must stay silent): pointers only

`main.c`

```c
int x;
void *q = &x;
```

### Violation (must be reported): integer cast to void pointer

`main.c`

```c
void *p = (void *)5; /*@V*/
```

### Violation (must be reported): void pointer cast to integer

`main.c`

```c
unsigned long f(void *p) { return (unsigned long)p; } /*@V*/
```

