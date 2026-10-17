# Rule 2.7

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Unused and unreachable code

## What the detector reports

Reports a named parameter of a function definition that the body never refers to.

Detection contract: Report a named parameter of a function definition that is never referenced in the body.

## Known limits

A parameter referenced only inside a macro that is not expanded counts as unused. Function declarations (without a body) are not examined.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | all parameters used | pass |
| Compliant (must stay silent) | a void cast counts as a reference | pass |
| Compliant (must stay silent) | a prototype never uses its parameters | pass |
| Violation (must be reported) | one parameter unused | pass |
| Violation (must be reported) | all parameters unused | pass |
| Compliant (must stay silent) | a parameter used only in sizeof | pass |
| Compliant (must stay silent) | a parameter used only in a condition | pass |
| Violation (must be reported) | a parameter shadowed by a local and never used | pass |

## Case sources

### Compliant (must stay silent): all parameters used

`main.c`

```c
int add(int a, int b) { return a + b; }
```

### Compliant (must stay silent): a void cast counts as a reference

`main.c`

```c
int pick(int a, int b) { (void)b; return a; }
```

### Compliant (must stay silent): a prototype never uses its parameters

`main.c`

```c
int proto(int a, int b);
```

### Violation (must be reported): one parameter unused

`main.c`

```c
int first(int a, int b) { return a; } /*@V*/
```

### Violation (must be reported): all parameters unused

`main.c`

```c
void nothing(int a, char *b) { } /*@V*/
```

### Compliant (must stay silent): a parameter used only in sizeof

`main.c`

```c
unsigned long f(int *p) { return sizeof(*p); }
```

### Compliant (must stay silent): a parameter used only in a condition

`main.c`

```c
int f(int a, int b) { if (b) { return a; } return 0; }
```

### Violation (must be reported): a parameter shadowed by a local and never used

`main.c`

```c
int f(int a) /*@V*/
{
    int a2 = 1;
    return a2;
}
```

