# Rule 20.5

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports every #undef directive.

Detection contract: Report every #undef directive.

## Known limits

None; every #undef is reported.

## Tested cases

5 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no #undef | pass |
| Violation (must be reported) | #undef of a macro | pass |
| Violation (must be reported) | #undef of a name that was never defined | pass |
| Violation (must be reported) | #undef inside a function | pass |
| Compliant (must stay silent) | only #define and #if | pass |

## Case sources

### Compliant (must stay silent): no #undef

`main.c`

```c
#define LIMIT 10
int a[LIMIT];
```

### Violation (must be reported): #undef of a macro

`main.c`

```c
#define LIMIT 10
int a[LIMIT];
#undef LIMIT /*@V*/
```

### Violation (must be reported): #undef of a name that was never defined

`main.c`

```c
#undef NEVER_DEFINED /*@V*/
```

### Violation (must be reported): #undef inside a function

`main.c`

```c
#define LEVEL 1
void f(void)
{
#undef LEVEL /*@V*/
}
```

### Compliant (must stay silent): only #define and #if

`main.c`

```c
#define LEVEL 1
#if LEVEL
int x;
#endif
```

