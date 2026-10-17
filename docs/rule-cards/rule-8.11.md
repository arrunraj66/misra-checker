# Rule 8.11

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports an extern array declaration without an explicit size.

Detection contract: Report an extern array declaration with no explicit size.

## Known limits

Array parameters (which decay to pointers) and defined arrays are not extern declarations and are accepted.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | extern array with its size | pass |
| Compliant (must stay silent) | an array parameter without a size | pass |
| Compliant (must stay silent) | a defined array sized by its initializer | pass |
| Violation (must be reported) | extern array of unspecified size | pass |
| Violation (must be reported) | extern two-dimensional array with the first size missing | pass |

## Case sources

### Compliant (must stay silent): extern array with its size

`main.c`

```c
extern int table[10];
```

### Compliant (must stay silent): an array parameter without a size

`main.c`

```c
int sum(int values[], int n);
```

### Compliant (must stay silent): a defined array sized by its initializer

`main.c`

```c
int primes[] = { 2, 3, 5 };
```

### Violation (must be reported): extern array of unspecified size

`main.c`

```c
extern int table[]; /*@V*/
```

### Violation (must be reported): extern two-dimensional array with the first size missing

`main.c`

```c
extern int grid[][4]; /*@V*/
```

