# Rule 5.1

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Identifiers

## What the detector reports

Reports external identifiers (functions and objects with external linkage) that are longer than 31 characters and identical in their first 31 characters.

Detection contract: Report external identifiers longer than 31 characters that are identical in their first 31 characters but differ afterwards (translation-unit view).

## Known limits

Only the translation-unit view is examined. 31 significant characters are used for external names in every language standard.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | external names that differ within 31 characters | pass |
| Compliant (must stay silent) | internal names are not external identifiers | pass |
| Compliant (must stay silent) | one object declared twice is one identifier | pass |
| Violation (must be reported) | two external objects identical in the first 31 characters | pass |
| Violation (must be reported) | two external functions identical in the first 31 characters | pass |

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

