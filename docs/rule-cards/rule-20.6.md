# Rule 20.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a preprocessing directive embedded in the arguments of a macro invocation (the compiler reports this as an extension warning).

Detection contract: Report a preprocessing directive embedded in macro arguments (reported by the compiler as an extension warning).

## Known limits

Found through the compiler's diagnostic, so the location is the directive inside the arguments.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | directives outside macro arguments | pass |
| Violation (must be reported) | a conditional directive inside macro arguments | pass |
| Compliant (must stay silent) | a multi-line macro invocation without directives | pass |

## Case sources

### Compliant (must stay silent): directives outside macro arguments

`main.c`

```c
#define F(x) (x)
#ifdef A
int a = F(1);
#else
int a = F(2);
#endif
```

### Violation (must be reported): a conditional directive inside macro arguments

`main.c`

```c
#define F(x) (x)
int v = F(
#ifdef A
    1
#else
    2
#endif
);
```

### Compliant (must stay silent): a multi-line macro invocation without directives

`main.c`

```c
#define ADD(a, b) ((a) + (b))
int x = ADD(1,
            2);
```

