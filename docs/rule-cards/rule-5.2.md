# Rule 5.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Identifiers

## What the detector reports

Reports identifiers declared in one scope that are identical in their significant characters (63 from C99, 31 in C90) and differ only afterwards.

Detection contract: Report identifiers in one scope longer than 31 characters that are identical in their first 31 characters but differ afterwards.

## Known limits

Only identifiers that exceed the significance limit are examined; the limit follows the language standard of the compilation.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | 32-character names are distinct in C99 | pass |
| Compliant (must stay silent) | names that differ early | pass |
| Compliant (must stay silent) | the same long name in two different functions | pass |
| Violation (must be reported) | 64-character names that differ only in the last character (C99) | pass |
| Violation (must be reported) | 32-character names that differ only after the 31st (C90 limit) | pass |

## Case sources

### Compliant (must stay silent): 32-character names are distinct in C99

`main.c`

```c
static int abcdefghijklmnopqrstuvwxyz12345a;
static int abcdefghijklmnopqrstuvwxyz12345b;
```

### Compliant (must stay silent): names that differ early

`main.c`

```c
static int first_counter_value;
static int second_counter_value;
```

### Compliant (must stay silent): the same long name in two different functions

`main.c`

```c
void f(void) { int abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz1234567890x = 1; (void)abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz1234567890x; }
void g(void) { int abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz1234567890y = 1; (void)abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz1234567890y; }
```

### Violation (must be reported): 64-character names that differ only in the last character (C99)

`main.c`

```c
static int aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaax;
static int aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaay;
```

### Violation (must be reported): 32-character names that differ only after the 31st (C90 limit)

`main.c`

```c
static int abcdefghijklmnopqrstuvwxyz12345a; /*@V*/
static int abcdefghijklmnopqrstuvwxyz12345b; /*@V*/
```

