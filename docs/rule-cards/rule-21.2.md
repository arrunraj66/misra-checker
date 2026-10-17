# Rule 21.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Standard library usage

## What the detector reports

Reports a declaration that uses a reserved identifier (leading underscore forms) or defines a function that has the name of a standard library function.

Detection contract: Report a declaration using an underscore-reserved identifier or defining a standard library function name.

## Known limits

Only the underscore forms and the standard function names listed in the checker are covered.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary names | pass |
| Compliant (must stay silent) | a local name with a leading underscore and a lowercase letter is not reserved | pass |
| Violation (must be reported) | file-scope names with leading underscores | pass |
| Violation (must be reported) | defining a standard library function | pass |

## Case sources

### Compliant (must stay silent): ordinary names

`main.c`

```c
int count;
int my_strlen(const char *s) { int n = 0; while (s[n] != 0) { n++; } return n; }
```

### Compliant (must stay silent): a local name with a leading underscore and a lowercase letter is not reserved

`main.c`

```c
void f(void) { int _local = 1; (void)_local; }
```

### Violation (must be reported): file-scope names with leading underscores

`main.c`

```c
int __internal; /*@V*/
int _Hidden; /*@V*/
```

### Violation (must be reported): defining a standard library function

`main.c`

```c
unsigned long strlen(const char *s) { return 0; } /*@V*/
```

