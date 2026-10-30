# Rule 4.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Character sets and lexical elements

## What the detector reports

Reports an octal or hexadecimal escape sequence that is not terminated: it must be followed by the closing quote or by another escape sequence (a complete three-digit octal escape counts as terminated).

Detection contract: Report an octal or hexadecimal escape sequence in a string or character literal that is not followed by the closing quote or another escape (a complete three-digit octal escape counts as terminated).

## Known limits

Only string and character literals are examined.

## Tested cases

12 of 12 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | terminated escapes | pass |
| Compliant (must stay silent) | simple escapes followed by text | pass |
| Violation (must be reported) | hexadecimal escape followed by a letter | pass |
| Violation (must be reported) | octal escape followed by a digit-like text | pass |
| Violation (must be reported) | hexadecimal escape followed by ordinary text | pass |
| Compliant (must stay silent) | adjacent literals terminate an escape | pass |
| Compliant (must stay silent) | octal escape with three digits followed by text | pass |
| Compliant (must stay silent) | nul escape at the end | pass |
| Violation (must be reported) | nul escape followed by a digit text | pass |
| Violation (must be reported) | octal escape followed by a letter | pass |
| Compliant (must stay silent) | character constants | pass |
| Violation (must be reported) | wide string hexadecimal escape followed by text | pass |

## Case sources

### Compliant (must stay silent): terminated escapes

`main.c`

```c
const char *a = "\x41";
const char *b = "\x41\x42";
const char *c = "\0";
const char *d = "\123";
const char *e = "plain\n";
char f = '\x41';
```

### Compliant (must stay silent): simple escapes followed by text

`main.c`

```c
const char *g = "tab\there";
```

### Violation (must be reported): hexadecimal escape followed by a letter

`main.c`

```c
const char *a = "\x41g"; /*@V*/
```

### Violation (must be reported): octal escape followed by a digit-like text

`main.c`

```c
const char *b = "\1x"; /*@V*/
```

### Violation (must be reported): hexadecimal escape followed by ordinary text

`main.c`

```c
const char *c = "\x0Dtext"; /*@V*/
```

### Compliant (must stay silent): adjacent literals terminate an escape

`main.c`

```c
const char *s = "\x41" "B";
```

### Compliant (must stay silent): octal escape with three digits followed by text

`main.c`

```c
const char *s = "\101B";
```

### Compliant (must stay silent): nul escape at the end

`main.c`

```c
const char s[] = "ab\0";
```

### Violation (must be reported): nul escape followed by a digit text

`main.c`

```c
const char *s = "\08"; /*@V*/
```

### Violation (must be reported): octal escape followed by a letter

`main.c`

```c
const char *s = "\1a"; /*@V*/
```

### Compliant (must stay silent): character constants

`main.c`

```c
int c = '\x41';
int d = '\101';
```

### Violation (must be reported): wide string hexadecimal escape followed by text

`main.c`

```c
#include <stddef.h>
const wchar_t *s = L"\x41g"; /*@V*/
```

