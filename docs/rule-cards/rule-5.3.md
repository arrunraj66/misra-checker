# Rule 5.3

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Identifiers

## What the detector reports

Reports a declaration in an inner scope (a block or a parameter) whose name matches an identifier of an enclosing scope.

Detection contract: Report a block-scope or parameter declaration whose name matches a file-scope or enclosing-scope identifier.

## Known limits

Names of struct members, labels and tags live in other name spaces and are not compared here.

## Tested cases

13 of 13 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | distinct names in nested scopes | pass |
| Compliant (must stay silent) | the same local name in two separate functions | pass |
| Compliant (must stay silent) | a struct member may share a name with a variable | pass |
| Violation (must be reported) | local variable hides a file-scope object | pass |
| Violation (must be reported) | parameter hides a file-scope object | pass |
| Violation (must be reported) | inner block variable hides an outer one | pass |
| Violation (must be reported) | for loop variable hides an outer variable | pass |
| Violation (must be reported) | local variable named like a function | pass |
| Compliant (must stay silent) | label and variable with the same name use different name spaces | pass |
| Compliant (must stay silent) | a tag and a variable share a name (different name space) | pass |
| Violation (must be reported) | parameter in a function definition hides a typedef-free global function | pass |
| Compliant (must stay silent) | sibling blocks reuse a name | pass |
| Violation (must be reported) | block variable hides a parameter | pass |

## Case sources

### Compliant (must stay silent): distinct names in nested scopes

`main.c`

```c
int global_count;
int f(int local_value) { int inner = local_value; return inner + global_count; }
```

### Compliant (must stay silent): the same local name in two separate functions

`main.c`

```c
int f(void) { int tmp = 1; return tmp; }
int g(void) { int tmp = 2; return tmp; }
```

### Compliant (must stay silent): a struct member may share a name with a variable

`main.c`

```c
int size;
struct S { int size; };
struct S s;
```

### Violation (must be reported): local variable hides a file-scope object

`main.c`

```c
int level;
int f(void)
{
    int level = 2; /*@V*/
    return level;
}
```

### Violation (must be reported): parameter hides a file-scope object

`main.c`

```c
int speed;
int f(int speed) /*@V*/
{
    return speed;
}
```

### Violation (must be reported): inner block variable hides an outer one

`main.c`

```c
int f(void)
{
    int value = 1;
    {
        int value = 2; /*@V*/
        return value;
    }
}
```

### Violation (must be reported): for loop variable hides an outer variable

`main.c`

```c
void f(void)
{
    int i = 0;
    for (int i = 0; i < 3; i++) { /*@V*/
        (void)i;
    }
    (void)i;
}
```

### Violation (must be reported): local variable named like a function

`main.c`

```c
int process(int a);
void g(void)
{
    int process = 1; /*@V*/
    (void)process;
}
```

### Compliant (must stay silent): label and variable with the same name use different name spaces

`main.c`

```c
void f(int a)
{
    if (a) { goto a2; }
a2:
    a = 1;
}
```

### Compliant (must stay silent): a tag and a variable share a name (different name space)

`main.c`

```c
struct item { int a; };
void f(void) { int item = 1; (void)item; }
```

### Violation (must be reported): parameter in a function definition hides a typedef-free global function

`main.c`

```c
int count(void);
int f(int count) { return count; } /*@V*/
```

### Compliant (must stay silent): sibling blocks reuse a name

`main.c`

```c
void f(int a)
{
    if (a) { int t = 1; (void)t; } else { int t = 2; (void)t; }
}
```

### Violation (must be reported): block variable hides a parameter

`main.c`

```c
int f(int n)
{
    {
        int n = 2; /*@V*/
        return n;
    }
}
```

