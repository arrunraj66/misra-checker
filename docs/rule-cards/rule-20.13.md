# Rule 20.13

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a line starting with # whose directive name is not a recognised preprocessing directive.

Detection contract: Report a line starting with # whose directive name is not a recognised preprocessing directive.

## Known limits

Checked textually, also in inactive groups.

## Tested cases

4 of 4 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | valid directives | pass |
| Compliant (must stay silent) | a null directive and a line marker are valid | pass |
| Violation (must be reported) | an unknown directive | pass |
| Violation (must be reported) | an unknown directive in an inactive group | pass |

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

