# Rule 8.3

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Declarations and definitions

## What the detector reports

Reports declarations of one external name whose type (qualifiers included) or parameter names differ from the first declaration seen.

Detection contract: Report declarations of one external name whose type (qualifiers included) or parameter names differ from the first declaration seen.

## Known limits

Only declarations in the analysed translation unit are compared; conflicts with a declaration in a header that is not visible are not found.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | identical declarations | pass |
| Compliant (must stay silent) | declarations of different functions | pass |
| Violation (must be reported) | parameter names differ between declaration and definition | pass |
| Violation (must be reported) | a top-level parameter qualifier differs (the compiler accepts it) | pass |

## Case sources

### Compliant (must stay silent): identical declarations

`main.c`

```c
int f(int a, int b);
int f(int a, int b) { return a + b; }
```

### Compliant (must stay silent): declarations of different functions

`main.c`

```c
int f(int a);
int g(int b);
```

### Violation (must be reported): parameter names differ between declaration and definition

`main.c`

```c
int f(int a);
int f(int b) { return b; } /*@V*/
```

### Violation (must be reported): a top-level parameter qualifier differs (the compiler accepts it)

`main.c`

```c
void put(const int count);
void put(int count) { } /*@V*/
```

