# Rule 20.13

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a line starting with # whose directive name is not a recognised preprocessing directive.

Detection contract: Report a line starting with # whose directive name is not a recognised preprocessing directive.

## Known limits

Checked textually, also in inactive groups.

- **False negative:** a warning directive is a common extension and is accepted (strict reading: not a C99 directive) (documented limit)

## Tested cases

6 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | valid directives | pass |
| Compliant (must stay silent) | a null directive and a line marker are valid | pass |
| Violation (must be reported) | an unknown directive | pass |
| Violation (must be reported) | an unknown directive in an inactive group | pass |
| Known false negative (not detected yet) | a warning directive is a common extension and is accepted (strict reading: not a C99 directive) | documented limit |
| Compliant (must stay silent) | a pragma | pass |
| Compliant (must stay silent) | an error directive in an inactive group is skipped | pass |

## Case sources

### Compliant (must stay silent): valid directives

`main.c`

```c
#define A 1
#if A
#endif
#pragma once
```

### Compliant (must stay silent): a null directive and a line marker are valid

`main.c`

```c
#
int a;
```

### Violation (must be reported): an unknown directive

`main.c`

```c
#foo /*@V*/
int a;
```

### Violation (must be reported): an unknown directive in an inactive group

`main.c`

```c
#if 0
#bar /*@V*/
#endif
int a;
```

### Known false negative (not detected yet): a warning directive is a common extension and is accepted (strict reading: not a C99 directive)

`main.c`

```c
#warning something /*@V*/
int a;
```

### Compliant (must stay silent): a pragma

`main.c`

```c
#pragma pack(1)
int a;
```

### Compliant (must stay silent): an error directive in an inactive group is skipped

`main.c`

```c
#if 0
#error never
#endif
int a;
```

