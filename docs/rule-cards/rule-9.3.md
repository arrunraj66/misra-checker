# Rule 9.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Initialization

## What the detector reports

Reports an array initializer list that supplies fewer elements than the array size, other than the single-zero form.

Detection contract: Report an array initializer list that supplies fewer elements than the array size, other than the single-zero form.

## Known limits

String literal initializers of char arrays are not examined here.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | every element supplied | pass |
| Compliant (must stay silent) | the single zero form | pass |
| Compliant (must stay silent) | size taken from the initializer | pass |
| Violation (must be reported) | fewer elements than the array size | pass |
| Violation (must be reported) | a single non-zero element for a larger array | pass |

## Case sources

### Compliant (must stay silent): every element supplied

`main.c`

```c
int a[3] = { 1, 2, 3 };
```

### Compliant (must stay silent): the single zero form

`main.c`

```c
int b[10] = { 0 };
```

### Compliant (must stay silent): size taken from the initializer

`main.c`

```c
int c[] = { 1, 2, 3, 4 };
```

### Violation (must be reported): fewer elements than the array size

`main.c`

```c
int a[5] = { 1, 2 }; /*@V*/
```

### Violation (must be reported): a single non-zero element for a larger array

`main.c`

```c
int b[4] = { 7 }; /*@V*/
```

