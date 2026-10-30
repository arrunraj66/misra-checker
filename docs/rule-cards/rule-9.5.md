# Rule 9.5

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Initialization

## What the detector reports

Reports an array of unspecified size that is initialized with array designators.

Detection contract: Report an array of unspecified size initialized with array designators.

## Known limits

Only arrays declared without a size are examined.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | sized array with designators | pass |
| Compliant (must stay silent) | unspecified size without designators | pass |
| Violation (must be reported) | unspecified size with a designator | pass |
| Compliant (must stay silent) | designators with an explicit size | pass |
| Violation (must be reported) | two array designators and no size | pass |
| Compliant (must stay silent) | a string initialiser without designators | pass |
| Violation (must be reported) | a designator in a nested unspecified array | pass |

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

### Compliant (must stay silent): designators with an explicit size

`main.c`

```c
int a[4] = { [1] = 1, [3] = 2 };
```

### Violation (must be reported): two array designators and no size

`main.c`

```c
int a[] = { [1] = 1, [3] = 2 }; /*@V*/
```

### Compliant (must stay silent): a string initialiser without designators

`main.c`

```c
char s[] = "abc";
```

### Violation (must be reported): a designator in a nested unspecified array

`main.c`

```c
int a[][2] = { [1] = { 1, 2 } }; /*@V*/
```

