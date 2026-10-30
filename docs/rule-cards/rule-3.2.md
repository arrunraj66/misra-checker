# Rule 3.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Comments

## What the detector reports

Reports a line comment that is continued onto the next line by a trailing backslash.

Detection contract: Report a line comment that is continued onto the next line by a trailing backslash.

## Known limits

Only line comments are examined.

## Tested cases

7 of 7 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | line comment without a continuation | pass |
| Compliant (must stay silent) | block comment may end a line with a backslash | pass |
| Violation (must be reported) | line comment continued by a backslash (the next line becomes part of the comment) | pass |
| Compliant (must stay silent) | a line comment that ends with a backslash inside a string | pass |
| Compliant (must stay silent) | backslash followed by a space is not a continuation of a block comment | pass |
| Violation (must be reported) | a trailing backslash after a line comment text | pass |
| Compliant (must stay silent) | a backslash in the middle of a line comment | pass |

## Case sources

### Compliant (must stay silent): line comment without a continuation

`main.c`

```c
// plain
int a;
```

### Compliant (must stay silent): block comment may end a line with a backslash

`main.c`

```c
/* text \
   continues */
int b;
```

### Violation (must be reported): line comment continued by a backslash (the next line becomes part of the comment)

`main.c`

```c
// continued comment \
int hidden_by_comment;
int c;
```

### Compliant (must stay silent): a line comment that ends with a backslash inside a string

`main.c`

```c
const char *s = "a\\";
int a;
```

### Compliant (must stay silent): backslash followed by a space is not a continuation of a block comment

`main.c`

```c
/* text \ */
int a;
```

### Violation (must be reported): a trailing backslash after a line comment text

`main.c`

```c
// note \
int hidden;
int x;
```

### Compliant (must stay silent): a backslash in the middle of a line comment

`main.c`

```c
// a \ b
int x;
```

