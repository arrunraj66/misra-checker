# Rule 8.14

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports any object, parameter or member declared with a restrict-qualified type.

Detection contract: Report any object, parameter or member with a restrict-qualified type.

## Known limits

The qualifier is reported wherever it is written.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no restrict | pass |
| Violation (must be reported) | restrict parameters | pass |
| Violation (must be reported) | restrict pointer object | pass |

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

