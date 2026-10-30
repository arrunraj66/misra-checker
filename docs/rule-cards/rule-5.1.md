# Rule 5.1

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Identifiers

## What the detector reports

Reports external identifiers (functions and objects with external linkage) that are longer than 31 characters and identical in their first 31 characters.

Detection contract: Report external identifiers longer than 31 characters that are identical in their first 31 characters but differ afterwards (translation-unit view).

## Known limits

Only the translation-unit view is examined. 31 significant characters are used for external names in every language standard.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | external names that differ within 31 characters | pass |
| Compliant (must stay silent) | internal names are not external identifiers | pass |
| Compliant (must stay silent) | one object declared twice is one identifier | pass |
| Violation (must be reported) | two external objects identical in the first 31 characters | pass |
| Violation (must be reported) | two external functions identical in the first 31 characters | pass |
| Compliant (must stay silent) | external names differing exactly at the 31st character | pass |
| Violation (must be reported) | external names identical in 31 characters, one an object and one a function | pass |
| Compliant (must stay silent) | long internal names are Rule 5.2 matters, not 5.1 | pass |
| Compliant (must stay silent) | a long external name that is unique | pass |

## Case sources

### Compliant (must stay silent): external names that differ within 31 characters

`main.c`

```c
int abcdefghijklmnopqrstuvwxyz12345;
int bbcdefghijklmnopqrstuvwxyz12345x;
```

### Compliant (must stay silent): internal names are not external identifiers

`main.c`

```c
static int abcdefghijklmnopqrstuvwxyz12345a;
static int abcdefghijklmnopqrstuvwxyz12345b;
```

### Compliant (must stay silent): one object declared twice is one identifier

`main.c`

```c
extern int abcdefghijklmnopqrstuvwxyz12345a;
int abcdefghijklmnopqrstuvwxyz12345a;
```

### Violation (must be reported): two external objects identical in the first 31 characters

`main.c`

```c
int abcdefghijklmnopqrstuvwxyz12345a; /*@V*/
int abcdefghijklmnopqrstuvwxyz12345b; /*@V*/
```

### Violation (must be reported): two external functions identical in the first 31 characters

`main.c`

```c
void abcdefghijklmnopqrstuvwxyz12345a(void) { } /*@V*/
void abcdefghijklmnopqrstuvwxyz12345b(void) { } /*@V*/
```

### Compliant (must stay silent): external names differing exactly at the 31st character

`main.c`

```c
int abcdefghijklmnopqrstuvwxyz0123a;
int abcdefghijklmnopqrstuvwxyz0123b;
```

### Violation (must be reported): external names identical in 31 characters, one an object and one a function

`main.c`

```c
int abcdefghijklmnopqrstuvwxyz01234_x;
int abcdefghijklmnopqrstuvwxyz01234_y(void) { return 0; }
```

### Compliant (must stay silent): long internal names are Rule 5.2 matters, not 5.1

`main.c`

```c
static int abcdefghijklmnopqrstuvwxyz01234_x;
static int abcdefghijklmnopqrstuvwxyz01234_y;
```

### Compliant (must stay silent): a long external name that is unique

`main.c`

```c
int a_very_long_external_identifier_name_that_is_unique_a;
int b_very_long_external_identifier_name_that_is_unique_b;
```

