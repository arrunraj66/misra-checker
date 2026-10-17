# Rule 3.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Comments

## What the detector reports

Reports a comment whose text contains a comment-opening marker (slash-star or double slash).

Detection contract: Report a comment whose body contains a comment-opening marker (slash-star or double slash).

## Known limits

A web address inside a block comment contains a double slash and is reported (strict reading). Comments inside string literals are not comments.

## Tested cases

6 of 6 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary comments | pass |
| Compliant (must stay silent) | the markers appear only inside a string literal | pass |
| Compliant (must stay silent) | a block comment that contains a single slash | pass |
| Violation (must be reported) | block comment containing a slash-star | pass |
| Violation (must be reported) | block comment containing a double slash | pass |
| Violation (must be reported) | web address inside a block comment (strict reading) | pass |

## Case sources

### Compliant (must stay silent): ordinary comments

`main.c`

```c
/* a normal comment */
// a line comment
int a; /* trailing comment */
```

### Compliant (must stay silent): the markers appear only inside a string literal

`main.c`

```c
const char *s = "/* not a comment */";
```

### Compliant (must stay silent): a block comment that contains a single slash

`main.c`

```c
/* path a/b and 4/2 */
int b;
```

### Violation (must be reported): block comment containing a slash-star

`main.c`

```c
/* outer /* inner */ /*@V*/
int a;
```

### Violation (must be reported): block comment containing a double slash

`main.c`

```c
/* see // note */ /*@V*/
int a;
```

### Violation (must be reported): web address inside a block comment (strict reading)

`main.c`

```c
/* docs: http://example.com/page */ /*@V*/
int a;
```

