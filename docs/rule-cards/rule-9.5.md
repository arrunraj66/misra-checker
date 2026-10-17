# Rule 9.5

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Initialization

## What the detector reports

Reports an array of unspecified size that is initialized with array designators.

Detection contract: Report an array of unspecified size initialized with array designators.

## Known limits

Only arrays declared without a size are examined.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | sized array with designators | pass |
| Compliant (must stay silent) | unspecified size without designators | pass |
| Violation (must be reported) | unspecified size with a designator | pass |

## Case sources

### Compliant (must stay silent): sized array with designators

`main.c`

```c
int a[5] = { [3] = 1 };
```

### Compliant (must stay silent): unspecified size without designators

`main.c`

```c
int b[] = { 1, 2, 3 };
```

### Violation (must be reported): unspecified size with a designator

`main.c`

```c
int a[] = { [2] = 1 }; /*@V*/
```

