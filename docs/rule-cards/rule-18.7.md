# Rule 18.7

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointers and arrays

## What the detector reports

Reports a structure whose last member is an array of unspecified size (a flexible array member).

Detection contract: Report a structure whose last member is an array of unspecified size.

## Known limits

Only structure definitions are examined.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | fixed-size array members | pass |
| Compliant (must stay silent) | a pointer member | pass |
| Violation (must be reported) | flexible array member | pass |
| Compliant (must stay silent) | the old one-element array idiom is a fixed array | pass |
| Violation (must be reported) | flexible array member after several members | pass |
| Compliant (must stay silent) | a pointer member where a flexible array would be | pass |
| Violation (must be reported) | a flexible array of structures | pass |

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

### Compliant (must stay silent): the old one-element array idiom is a fixed array

`main.c`

```c
struct s { int n; int d[1]; };
```

### Violation (must be reported): flexible array member after several members

`main.c`

```c
struct s { int n; char tag; unsigned char d[]; }; /*@V*/
```

### Compliant (must stay silent): a pointer member where a flexible array would be

`main.c`

```c
struct s { int n; int *d; };
```

### Violation (must be reported): a flexible array of structures

`main.c`

```c
struct item { int a; };
struct bag { int n; struct item items[]; }; /*@V*/
```

