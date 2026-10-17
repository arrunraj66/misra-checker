# Rule 10.5

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Essential type model

## What the detector reports

Subset: reports casts to or from Boolean, to character, to an enumeration type from another type, and from character to floating.

Detection contract: Subset: report casts to or from Boolean, to character, to enumeration of another type, and character to floating.

## Known limits

Numeric casts between integer and floating types are not reported. Whether a cast of an integer constant to char is reported depends on the strict reading (undecided, see docs/independent-validation.md).

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | numeric casts | pass |
| Compliant (must stay silent) | a cast between character types | pass |
| Violation (must be reported) | unsigned char is an unsigned number in the essential type model, not a character | pass |
| Violation (must be reported) | integer cast to Boolean | pass |
| Violation (must be reported) | integer cast to character | pass |
| Violation (must be reported) | integer cast to an enumeration | pass |
| Violation (must be reported) | character cast to floating | pass |

## Case sources

### Compliant (must stay silent): numeric casts

`main.c`

```c
long f(int a) { return (long)a; }
unsigned int g(int a) { return (unsigned int)a; }
float h(int a) { return (float)a; }
```

### Compliant (must stay silent): a cast between character types

`main.c`

```c
char k(char c) { return (char)c; }
```

### Violation (must be reported): unsigned char is an unsigned number in the essential type model, not a character

`main.c`

```c
char u(unsigned char c) { return (char)c; } /*@V*/
```

### Violation (must be reported): integer cast to Boolean

`main.c`

```c
_Bool f(int a) { return (_Bool)a; } /*@V*/
```

### Violation (must be reported): integer cast to character

`main.c`

```c
char g(int a) { return (char)a; } /*@V*/
```

### Violation (must be reported): integer cast to an enumeration

`main.c`

```c
enum e { A, B };
enum e h(int a) { return (enum e)a; } /*@V*/
```

### Violation (must be reported): character cast to floating

`main.c`

```c
float k(char c) { return (float)c; } /*@V*/
```

