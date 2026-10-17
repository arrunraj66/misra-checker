# Rule 18.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointers and arrays

## What the detector reports

Reports a structure whose last member is an array of unspecified size (a flexible array member).

Detection contract: Report a structure whose last member is an array of unspecified size.

## Known limits

Only structure definitions are examined.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | fixed-size array members | pass |
| Compliant (must stay silent) | a pointer member | pass |
| Violation (must be reported) | flexible array member | pass |

## Case sources

### Compliant (must stay silent): fixed-size array members

`main.c`

```c
struct a { int n; int data[4]; };
```

### Compliant (must stay silent): a pointer member

`main.c`

```c
struct b { int n; int *data; };
```

### Violation (must be reported): flexible array member

`main.c`

```c
struct c { int n; int data[]; }; /*@V*/
```

