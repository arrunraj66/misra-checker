# Rule 5.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Identifiers

## What the detector reports

Reports macro names that exceed the significance limit (63 characters from C99, 31 in C90) and differ only after it.

Detection contract: Report macro names longer than 31 characters that differ only after the 31st character.

## Known limits

Only macros defined in project code are examined.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | long macro names that differ early | pass |
| Compliant (must stay silent) | 32-character macro names are distinct in C99 | pass |
| Violation (must be reported) | 64-character macro names that differ only in the last character | pass |
| Violation (must be reported) | 32-character macro names differ only after the 31st (C90 limit) | pass |

## Case sources

### Compliant (must stay silent): long macro names that differ early

`main.c`

```c
#define MACRO_NUMBER_ONE_WITH_A_VERY_LONG_NAME_THAT_KEEPS_GOING_AND_GOING_X 1
#define MACRO_NUMBER_TWO_WITH_A_VERY_LONG_NAME_THAT_KEEPS_GOING_AND_GOING_X 2
int v = MACRO_NUMBER_ONE_WITH_A_VERY_LONG_NAME_THAT_KEEPS_GOING_AND_GOING_X + MACRO_NUMBER_TWO_WITH_A_VERY_LONG_NAME_THAT_KEEPS_GOING_AND_GOING_X;
```

### Compliant (must stay silent): 32-character macro names are distinct in C99

`main.c`

```c
#define abcdefghijklmnopqrstuvwxyz12345a 1
#define abcdefghijklmnopqrstuvwxyz12345b 2
int v = abcdefghijklmnopqrstuvwxyz12345a + abcdefghijklmnopqrstuvwxyz12345b;
```

### Violation (must be reported): 64-character macro names that differ only in the last character

`main.c`

```c
#define aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaax 1 /*@V*/
#define aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaay 2 /*@V*/
int v = aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaax + aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaay;
```

### Violation (must be reported): 32-character macro names differ only after the 31st (C90 limit)

`main.c`

```c
#define abcdefghijklmnopqrstuvwxyz12345a 1 /*@V*/
#define abcdefghijklmnopqrstuvwxyz12345b 2 /*@V*/
int v = abcdefghijklmnopqrstuvwxyz12345a + abcdefghijklmnopqrstuvwxyz12345b;
```

