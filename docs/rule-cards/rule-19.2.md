# Rule 19.2

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Overlapping storage

## What the detector reports

Reports the definition of a union type.

Detection contract: Report the definition of a union type.

## Known limits

Only definitions are reported, not uses of a union defined elsewhere.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | structures only | pass |
| Violation (must be reported) | a union definition | pass |
| Violation (must be reported) | an anonymous union member | pass |
| Compliant (must stay silent) | a use of a union defined elsewhere is not a definition | pass |
| Violation (must be reported) | a typedef of a union | pass |

## Case sources

### Compliant (must stay silent): structures only

`main.c`

```c
struct s { int a; float b; };
```

### Violation (must be reported): a union definition

`main.c`

```c
union u { int i; float f; }; /*@V*/
```

### Violation (must be reported): an anonymous union member

`main.c`

```c
struct s { int kind; union { int i; float f; } value; }; /*@V*/
```

### Compliant (must stay silent): a use of a union defined elsewhere is not a definition

`main.c`

```c
struct s { int kind; };
```

### Violation (must be reported): a typedef of a union

`main.c`

```c
typedef union { int i; float f; } mix_t; /*@V*/
```

