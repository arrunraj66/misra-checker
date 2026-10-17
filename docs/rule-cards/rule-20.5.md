# Rule 20.5

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports every #undef directive.

Detection contract: Report every #undef directive.

## Known limits

None; every #undef is reported.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | no #undef | pass |
| Violation (must be reported) | #undef of a macro | pass |
| Violation (must be reported) | #undef of a name that was never defined | pass |

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

