# Rule 20.4

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a macro whose name is a language keyword.

Detection contract: Report a macro whose name is a language keyword.

## Known limits

Placed last in the file in these cases, because redefining a keyword changes the code after it.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary macro names | pass |
| Violation (must be reported) | a macro named like a keyword | pass |
| Violation (must be reported) | a macro named int | pass |
| Violation (must be reported) | a macro named restrict | pass |
| Violation (must be reported) | a macro named while | pass |
| Compliant (must stay silent) | bool as a macro name (not a C99 keyword) | pass |
| Compliant (must stay silent) | a macro name containing a keyword | pass |

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

### Violation (must be reported): a macro named restrict

`main.c`

```c
#define restrict /*@V*/
int x;
```

### Violation (must be reported): a macro named while

`main.c`

```c
#define while(x) if (x) /*@V*/
int y;
```

### Compliant (must stay silent): bool as a macro name (not a C99 keyword)

`main.c`

```c
#define bool int
bool x;
```

### Compliant (must stay silent): a macro name containing a keyword

`main.c`

```c
#define my_int_type int
my_int_type x;
```

