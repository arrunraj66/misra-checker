# Rule 9.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Initialization

## What the detector reports

Reports an array initializer list that supplies fewer elements than the array size, other than the single-zero form.

Detection contract: Report an array initializer list that supplies fewer elements than the array size, other than the single-zero form.

## Known limits

String literal initializers of char arrays are not examined here.

## Tested cases

10 of 10 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | every element supplied | pass |
| Compliant (must stay silent) | the single zero form | pass |
| Compliant (must stay silent) | size taken from the initializer | pass |
| Violation (must be reported) | fewer elements than the array size | pass |
| Violation (must be reported) | a single non-zero element for a larger array | pass |
| Violation (must be reported) | two of three elements | pass |
| Compliant (must stay silent) | designated initialisers | pass |
| Compliant (must stay silent) | a string initialiser shorter than the array | pass |
| Compliant (must stay silent) | a structure initialised partially (arrays only for this rule) | pass |
| Violation (must be reported) | nested array row incomplete | pass |

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

### Violation (must be reported): two of three elements

`main.c`

```c
int a[3] = { 1, 2 }; /*@V*/
```

### Compliant (must stay silent): designated initialisers

`main.c`

```c
int a[3] = { [0] = 1, [1] = 2, [2] = 3 };
```

### Compliant (must stay silent): a string initialiser shorter than the array

`main.c`

```c
char s[8] = "ab";
```

### Compliant (must stay silent): a structure initialised partially (arrays only for this rule)

`main.c`

```c
struct s { int a; int b; };
struct s v = { 1 };
```

### Violation (must be reported): nested array row incomplete

`main.c`

```c
int m[2][2] = { { 1, 2 }, { 3 } }; /*@V*/
```

