# Rule 20.8

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Subset: reports an #if or #elif condition without identifiers that evaluates to a value other than 0 or 1.

Detection contract: Subset: report an identifier-free #if/#elif condition that evaluates to a value other than 0 or 1.

## Known limits

Only identifier-free conditions are evaluated.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | conditions that are 0 or 1 | pass |
| Violation (must be reported) | a condition that evaluates to 2 | pass |
| Violation (must be reported) | a condition that evaluates to a larger value | pass |
| Violation (must be reported) | a condition with arithmetic | pass |
| Violation (must be reported) | a negative value | pass |
| Compliant (must stay silent) | a comparison that yields one | pass |
| Compliant (must stay silent) | defined and logical operators | pass |

## Case sources

### Compliant (must stay silent): conditions that are 0 or 1

`main.c`

```c
#if 1
int a;
#endif
#if 0
int b;
#endif
#if (1 + 1) == 2
int c;
#endif
```

### Violation (must be reported): a condition that evaluates to 2

`main.c`

```c
#if 2 /*@V*/
int a;
#endif
```

### Violation (must be reported): a condition that evaluates to a larger value

`main.c`

```c
#if (3 * 4) /*@V*/
int a;
#endif
```

### Violation (must be reported): a condition with arithmetic

`main.c`

```c
#if 1 + 1 /*@V*/
int a;
#endif
```

### Violation (must be reported): a negative value

`main.c`

```c
#if -1 /*@V*/
int a;
#endif
```

### Compliant (must stay silent): a comparison that yields one

`main.c`

```c
#if 3 > 2
int a;
#endif
```

### Compliant (must stay silent): defined and logical operators

`main.c`

```c
#define A
#if defined(A) && !defined(B)
int a;
#endif
```

