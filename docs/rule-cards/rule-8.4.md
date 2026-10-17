# Rule 8.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports the definition of an externally visible function or object that has no earlier declaration in the translation unit (main is exempt).

Detection contract: Report a definition of an externally visible function or object that has no earlier declaration (main excluded).

## Known limits

A declaration in a header counts as earlier only when it is visible in the same translation unit; whether the declarations are compatible is Rule 8.3.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | prototype before the definition | pass |
| Compliant (must stay silent) | static functions and objects need no declaration | pass |
| Compliant (must stay silent) | main is exempt | pass |
| Compliant (must stay silent) | object declared extern before its definition | pass |
| Violation (must be reported) | function defined without a prior declaration | pass |
| Violation (must be reported) | object defined without a prior declaration | pass |
| Violation (must be reported) | a prototype that appears after the definition does not help | pass |

## Case sources

### Compliant (must stay silent): prototype before the definition

`main.c`

```c
int get(void);
int get(void) { return 1; }
```

### Compliant (must stay silent): static functions and objects need no declaration

`main.c`

```c
static int hidden(void) { return 1; }
static int counter;
```

### Compliant (must stay silent): main is exempt

`main.c`

```c
int main(void) { return 0; }
```

### Compliant (must stay silent): object declared extern before its definition

`main.c`

```c
extern int level;
int level = 3;
```

### Violation (must be reported): function defined without a prior declaration

`main.c`

```c
int compute(int a) /*@V*/
{
    return a;
}
```

### Violation (must be reported): object defined without a prior declaration

`main.c`

```c
int total = 0; /*@V*/
```

### Violation (must be reported): a prototype that appears after the definition does not help

`main.c`

```c
int late(void) { return 1; } /*@V*/
int late(void);
```

