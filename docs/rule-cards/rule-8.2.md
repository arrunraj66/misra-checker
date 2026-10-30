# Rule 8.2

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Declarations and definitions

## What the detector reports

Reports a function declared without a prototype (an empty parameter list) or with an unnamed parameter.

Detection contract: Report a function declared without a prototype or with an unnamed parameter.

## Known limits

The unnamed-parameter form is reported on declarations and on definitions alike.

- **False negative:** a function pointer type with unnamed parameters (only function declarations are checked) (documented limit)

## Tested cases

12 of 13 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | prototypes with named parameters | pass |
| Compliant (must stay silent) | a definition with named parameters | pass |
| Violation (must be reported) | an empty parameter list is not a prototype | pass |
| Violation (must be reported) | an unnamed parameter | pass |
| Violation (must be reported) | a definition in the old style | pass |
| Violation (must be reported) | a prototype with only a type | pass |
| Compliant (must stay silent) | a variadic prototype with a named first parameter | pass |
| Compliant (must stay silent) | array and function pointer parameters with names | pass |
| Compliant (must stay silent) | void parameter list | pass |
| Violation (must be reported) | a definition with an empty parameter list | pass |
| Compliant (must stay silent) | a function pointer type with named parameters | pass |
| Compliant (must stay silent) | a function pointer object that takes no parameters | pass |
| Known false negative (not detected yet) | a function pointer type with unnamed parameters (only function declarations are checked) | documented limit |

## Case sources

### Compliant (must stay silent): prototypes with named parameters

`main.c`

```c
int add(int a, int b);
void nothing(void);
```

### Compliant (must stay silent): a definition with named parameters

`main.c`

```c
int add(int a, int b) { return a + b; }
```

### Violation (must be reported): an empty parameter list is not a prototype

`main.c`

```c
int f(); /*@V*/
```

### Violation (must be reported): an unnamed parameter

`main.c`

```c
int g(int); /*@V*/
```

### Violation (must be reported): a definition in the old style

`main.c`

```c
int h() { return 0; } /*@V*/
```

### Violation (must be reported): a prototype with only a type

`main.c`

```c
void f(int); /*@V*/
```

### Compliant (must stay silent): a variadic prototype with a named first parameter

`main.c`

```c
int p(const char *fmt, ...);
```

### Compliant (must stay silent): array and function pointer parameters with names

`main.c`

```c
void f(int a[], int (*cb)(int x));
```

### Compliant (must stay silent): void parameter list

`main.c`

```c
int f(void);
```

### Violation (must be reported): a definition with an empty parameter list

`main.c`

```c
int f() { return 0; } /*@V*/
```

### Compliant (must stay silent): a function pointer type with named parameters

`main.c`

```c
typedef int (*op_t)(int a, int b);
op_t table[2];
```

### Compliant (must stay silent): a function pointer object that takes no parameters

`main.c`

```c
int (*fp)(void);
```

### Known false negative (not detected yet): a function pointer type with unnamed parameters (only function declarations are checked)

`main.c`

```c
typedef int (*op_t)(int, int); /*@V*/
op_t table[2];
```

