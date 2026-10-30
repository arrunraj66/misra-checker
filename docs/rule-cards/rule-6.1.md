# Rule 6.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Types and bit-fields

## What the detector reports

Reports a bit-field whose type is not int, unsigned int or _Bool.

Detection contract: Report a bit-field whose canonical type is not int, unsigned int or _Bool.

## Known limits

The implementation-defined types a compiler may allow are not accepted unless they are one of those three.

## Tested cases

9 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | the permitted bit-field types | pass |
| Violation (must be reported) | a char bit-field | pass |
| Violation (must be reported) | a short and a long bit-field | pass |
| Violation (must be reported) | an enumerated bit-field | pass |
| Compliant (must stay silent) | typedef of unsigned int used as a bit-field | pass |
| Compliant (must stay silent) | plain unsigned and signed int bit-fields | pass |
| Violation (must be reported) | an unsigned char bit-field | pass |
| Compliant (must stay silent) | zero-width unnamed bit-field | pass |
| Compliant (must stay silent) | a boolean bit-field | pass |

## Case sources

### Compliant (must stay silent): the permitted bit-field types

`main.c`

```c
struct flags { unsigned int a : 3; int b : 4; _Bool c : 1; };
struct flags f;
```

### Violation (must be reported): a char bit-field

`main.c`

```c
struct s1 { unsigned char a : 3; } v1; /*@V*/
```

### Violation (must be reported): a short and a long bit-field

`main.c`

```c
struct s2
{
    unsigned short a : 3; /*@V*/
    long b : 5; /*@V*/
} v2;
```

### Violation (must be reported): an enumerated bit-field

`main.c`

```c
enum e { E1, E2 };
struct s3 { enum e a : 2; } v3; /*@V*/
```

### Compliant (must stay silent): typedef of unsigned int used as a bit-field

`main.c`

```c
typedef unsigned int bits_t;
struct s { bits_t a : 3; };
```

### Compliant (must stay silent): plain unsigned and signed int bit-fields

`main.c`

```c
struct s { unsigned a : 3; signed int b : 3; };
```

### Violation (must be reported): an unsigned char bit-field

`main.c`

```c
struct s { unsigned char a : 3; }; /*@V*/
```

### Compliant (must stay silent): zero-width unnamed bit-field

`main.c`

```c
struct s { unsigned a : 3; unsigned : 0; unsigned b : 2; };
```

### Compliant (must stay silent): a boolean bit-field

`main.c`

```c
struct s { _Bool flag : 1; };
```

