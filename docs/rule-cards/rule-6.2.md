# Rule 6.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Types and bit-fields

## What the detector reports

Reports a named bit-field of signed integer type with a width of one bit.

Detection contract: Report a named bit-field of signed integer type with width one.

## Known limits

Unnamed bit-fields are not reported. Only signed integer types are examined.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | unsigned single-bit field | pass |
| Compliant (must stay silent) | signed field wider than one bit | pass |
| Compliant (must stay silent) | unnamed single-bit signed field (padding) | pass |
| Violation (must be reported) | named signed single-bit field | pass |
| Violation (must be reported) | two fields, one of them signed single-bit | pass |

## Case sources

### Compliant (must stay silent): unsigned single-bit field

`main.c`

```c
struct a { unsigned int flag : 1; } va;
```

### Compliant (must stay silent): signed field wider than one bit

`main.c`

```c
struct b { int level : 2; } vb;
```

### Compliant (must stay silent): unnamed single-bit signed field (padding)

`main.c`

```c
struct c { int : 1; unsigned int x : 3; } vc;
```

### Violation (must be reported): named signed single-bit field

`main.c`

```c
struct d { int flag : 1; } vd; /*@V*/
```

### Violation (must be reported): two fields, one of them signed single-bit

`main.c`

```c
struct e
{
    unsigned int ok_flag : 1;
    int bad_flag : 1; /*@V*/
} ve;
```

