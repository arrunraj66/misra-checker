# Rule 5.7

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Identifiers

## What the detector reports

Reports a tag name that is reused by another declaration (a typedef that names the same type is accepted). The finding is reported at the tag declaration.

Detection contract: Report a tag whose name is reused by another declaration (a typedef naming the same type is accepted).

## Known limits

Tags from system headers are not examined.

## Tested cases

6 of 6 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | tag and typedef naming the same type | pass |
| Compliant (must stay silent) | unique tags | pass |
| Violation (must be reported) | a variable reuses a tag name | pass |
| Violation (must be reported) | two different types with the same tag name in different scopes | pass |
| Violation (must be reported) | a function named like a tag | pass |
| Compliant (must stay silent) | a tag used in the tag name space only | pass |

## Case sources

### Compliant (must stay silent): tag and typedef naming the same type

`main.c`

```c
struct point { int x; };
typedef struct point point;
point p;
```

### Compliant (must stay silent): unique tags

`main.c`

```c
struct a_tag { int x; };
struct b_tag { int y; };
struct a_tag a;
struct b_tag b;
```

### Violation (must be reported): a variable reuses a tag name

`main.c`

```c
struct shape { int sides; }; /*@V*/
struct shape s;
int shape;
```

### Violation (must be reported): two different types with the same tag name in different scopes

`main.c`

```c
struct item { int a; };
struct item first;
void f(void)
{
    struct item { long b; }; /*@V*/
    struct item second;
    (void)second;
}
```

### Violation (must be reported): a function named like a tag

`main.c`

```c
struct node { int a; }; /*@V*/
int node(void) { return 1; }
```

### Compliant (must stay silent): a tag used in the tag name space only

`main.c`

```c
struct a { int x; };
struct b { struct a inner; };
```

