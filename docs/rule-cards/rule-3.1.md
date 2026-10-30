# Rule 3.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Comments

## What the detector reports

Reports a comment whose text contains a comment-opening marker (slash-star or double slash).

Detection contract: Report a comment whose body contains a comment-opening marker (slash-star or double slash).

## Known limits

A web address inside a block comment contains a double slash and is reported (strict reading). Comments inside string literals are not comments.

## Tested cases

13 of 13 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | ordinary comments | pass |
| Compliant (must stay silent) | the markers appear only inside a string literal | pass |
| Compliant (must stay silent) | a block comment that contains a single slash | pass |
| Violation (must be reported) | block comment containing a slash-star | pass |
| Violation (must be reported) | block comment containing a double slash | pass |
| Violation (must be reported) | web address inside a block comment (strict reading) | pass |
| Violation (must be reported) | line comment containing a block-comment opener | pass |
| Compliant (must stay silent) | two comments on one line | pass |
| Compliant (must stay silent) | comment ending with several stars | pass |
| Violation (must be reported) | a nested opener hidden at the end of a comment | pass |
| Compliant (must stay silent) | a division operator in code beside a comment | pass |
| Compliant (must stay silent) | slashes inside a character constant | pass |
| Violation (must be reported) | web address inside a line comment (strict reading) | pass |

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

### Violation (must be reported): line comment containing a block-comment opener

`main.c`

```c
int a; // see /* here */ /*@V*/
```

### Compliant (must stay silent): two comments on one line

`main.c`

```c
int a; /* one *//* two */
```

### Compliant (must stay silent): comment ending with several stars

`main.c`

```c
/*** banner ***/
int a;
```

### Violation (must be reported): a nested opener hidden at the end of a comment

`main.c`

```c
/* text /* */
int a;
```

### Compliant (must stay silent): a division operator in code beside a comment

`main.c`

```c
int f(int x, int y) { return x / y; /* quotient */ }
```

### Compliant (must stay silent): slashes inside a character constant

`main.c`

```c
int c = '/';
```

### Violation (must be reported): web address inside a line comment (strict reading)

`main.c`

```c
// see http://example.org
int a;
```

