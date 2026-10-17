# Rule 3.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Comments

## What the detector reports

Reports a line comment that is continued onto the next line by a trailing backslash.

Detection contract: Report a line comment that is continued onto the next line by a trailing backslash.

## Known limits

Only line comments are examined.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | line comment without a continuation | pass |
| Compliant (must stay silent) | block comment may end a line with a backslash | pass |
| Violation (must be reported) | line comment continued by a backslash (the next line becomes part of the comment) | pass |

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

