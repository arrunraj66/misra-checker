# Rule 5.8

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Identifiers

## What the detector reports

Whole-program check: reports an identifier with external linkage that is also used as an identifier with internal linkage elsewhere in the program.

Detection contract: Whole-program: report an external-linkage name that is also the name of an internal-linkage function or object elsewhere in the program (Inconclusive with a single translation unit).

## Known limits

Needs every translation unit of the program in one run; with a single translation unit the rule is not assessed.

## Tested cases

3 of 3 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | distinct names across files | pass |
| Violation (must be reported) | an external name that another file uses as a static name | pass |
| Violation (must be reported) | an external function whose name is a static function elsewhere | pass |

## Case sources

### Compliant (must stay silent): distinct names across files

`a.c`

```c
int shared_counter;
static int local_a;
int use_a(void) { return shared_counter + local_a; }
```

`b.c`

```c
extern int shared_counter;
static int local_b;
int use_b(void) { return shared_counter + local_b; }
```

### Violation (must be reported): an external name that another file uses as a static name

`a.c`

```c
int counter = 1;
int use_a(void) { return counter; }
```

`b.c`

```c
static int counter = 2; /*@V*/
int use_b(void) { return counter; }
```

### Violation (must be reported): an external function whose name is a static function elsewhere

`a.c`

```c
int helper(void) { return 1; }
```

`b.c`

```c
static int helper(void) { return 2; } /*@V*/
int use(void) { return helper(); }
```

