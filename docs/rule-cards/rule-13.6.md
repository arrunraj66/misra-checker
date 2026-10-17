# Rule 13.6

**Category:** Mandatory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Side effects and evaluation order

## What the detector reports

Reports a sizeof operand expression that has side effects.

Detection contract: Report a sizeof operand expression that has side effects.

## Known limits

Only the unevaluated expression form is examined.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | sizeof of types and of pure expressions | pass |
| Violation (must be reported) | increment inside sizeof | pass |
| Violation (must be reported) | assignment inside sizeof | pass |
| Violation (must be reported) | call inside sizeof | pass |

## Case sources

### Compliant (must stay silent): sizeof of types and of pure expressions

`main.c`

```c
unsigned long a = sizeof(int);
int arr[4];
unsigned long b = sizeof(arr);
unsigned long c(int i) { return sizeof(arr[i]); }
```

### Violation (must be reported): increment inside sizeof

`main.c`

```c
unsigned long f(int i) { return sizeof(i++); } /*@V*/
```

### Violation (must be reported): assignment inside sizeof

`main.c`

```c
unsigned long g(int i) { return sizeof(i = 2); } /*@V*/
```

### Violation (must be reported): call inside sizeof

`main.c`

```c
int next(void);
unsigned long h(void) { return sizeof(next()); } /*@V*/
```

