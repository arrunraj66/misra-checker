# Rule 20.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a macro whose name is a language keyword.

Detection contract: Report a macro whose name is a language keyword.

## Known limits

Placed last in the file in these cases, because redefining a keyword changes the code after it.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary macro names | pass |
| Violation (must be reported) | a macro named like a keyword | pass |
| Violation (must be reported) | a macro named int | pass |

## Case sources

### Compliant (must stay silent): ordinary macro names

`main.c`

```c
#define LIMIT 10
#define MY_INLINE static
```

### Violation (must be reported): a macro named like a keyword

`main.c`

```c
int a;
#define inline /*@V*/
```

### Violation (must be reported): a macro named int

`main.c`

```c
int a;
#define int long /*@V*/
```

