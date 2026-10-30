# Rule 8.12

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports an implicitly valued enumerator whose value equals the value of another enumerator in the same list.

Detection contract: Report an implicitly valued enumerator whose value equals another enumerator in the same list.

## Known limits

Explicit duplicate values are allowed; only an implicit value that coincides with another is reported.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | all implicit | pass |
| Compliant (must stay silent) | explicit values only | pass |
| Compliant (must stay silent) | the first is explicit and the rest follow implicitly without a clash | pass |
| Violation (must be reported) | an implicit value equals an earlier explicit one | pass |
| Violation (must be reported) | an implicit enumerator follows a value that makes it equal to another | pass |
| Violation (must be reported) | implicit enumerator equals a later explicit one | pass |
| Compliant (must stay silent) | all explicit and duplicated by choice | pass |
| Compliant (must stay silent) | negative start then implicit | pass |
| Violation (must be reported) | implicit equals the first explicit value of a negative start | pass |

## Case sources

### Compliant (must stay silent): all implicit

`main.c`

```c
enum a { A0, A1, A2 };
```

### Compliant (must stay silent): explicit values only

`main.c`

```c
enum b { B0 = 1, B1 = 5, B2 = 9 };
```

### Compliant (must stay silent): the first is explicit and the rest follow implicitly without a clash

`main.c`

```c
enum c { C0 = 5, C1, C2 };
```

### Violation (must be reported): an implicit value equals an earlier explicit one

`main.c`

```c
enum d { D0 = 1, D1 = 0, D2 }; /*@V*/
```

### Violation (must be reported): an implicit enumerator follows a value that makes it equal to another

`main.c`

```c
enum e { E0 = 2, E1 = 1, E2, E3 = 3 }; /*@V*/
```

### Violation (must be reported): implicit enumerator equals a later explicit one

`main.c`

```c
enum e { A = 0, B, C = 1 }; /*@V*/
```

### Compliant (must stay silent): all explicit and duplicated by choice

`main.c`

```c
enum e { A = 1, B = 1 };
```

### Compliant (must stay silent): negative start then implicit

`main.c`

```c
enum e { A = -1, B, C };
```

### Violation (must be reported): implicit equals the first explicit value of a negative start

`main.c`

```c
enum e { A, B = -1, C }; /*@V*/
```

