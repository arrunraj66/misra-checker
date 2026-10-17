# Rule 5.9

**Category:** Advisory · **Decidability:** Decidable · **Scope:** System · **Topic:** Identifiers

## What the detector reports

Whole-program check: reports internal-linkage functions or objects that share a name but are defined at different places in the program.

Detection contract: Whole-program: report internal-linkage functions or objects that share a name but are defined at different locations (Inconclusive with a single translation unit).

## Known limits

Needs every translation unit of the program in one run; with a single translation unit the rule is not assessed.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | static names that are distinct across files | pass |
| Violation (must be reported) | the same static name in two files | pass |
| Violation (must be reported) | the same static object name in two files | pass |

## Case sources

### Compliant (must stay silent): static names that are distinct across files

`a.c`

```c
static int helper_a(void) { return 1; }
int use_a(void) { return helper_a(); }
```

`b.c`

```c
static int helper_b(void) { return 2; }
int use_b(void) { return helper_b(); }
```

### Violation (must be reported): the same static name in two files

`a.c`

```c
static int helper(void) { return 1; } /*@V*/
int use_a(void) { return helper(); }
```

`b.c`

```c
static int helper(void) { return 2; } /*@V*/
int use_b(void) { return helper(); }
```

### Violation (must be reported): the same static object name in two files

`a.c`

```c
static int state = 1; /*@V*/
int get_a(void) { return state; }
```

`b.c`

```c
static int state = 2; /*@V*/
int get_b(void) { return state; }
```

