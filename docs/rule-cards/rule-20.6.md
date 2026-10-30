# Rule 20.6

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Preprocessing directives

## What the detector reports

Reports a preprocessing directive embedded in the arguments of a macro invocation (the compiler reports this as an extension warning).

Detection contract: Report a preprocessing directive embedded in macro arguments (reported by the compiler as an extension warning).

## Known limits

Found through the compiler's diagnostic, so the location is the directive inside the arguments.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | directives outside macro arguments | pass |
| Violation (must be reported) | a conditional directive inside macro arguments | pass |
| Compliant (must stay silent) | a multi-line macro invocation without directives | pass |
| Violation (must be reported) | #define inside macro arguments | pass |
| Violation (must be reported) | #ifdef inside macro arguments | pass |
| Compliant (must stay silent) | a directive after the invocation ends | pass |
| Compliant (must stay silent) | a hash character inside a string argument | pass |
| Compliant (must stay silent) | a directive inside a function body, outside any invocation | pass |

## Case sources

### Compliant (must stay silent): directives outside macro arguments

`main.c`

```c
#define F(x) (x)
#ifdef A
int a = F(1);
#else
int a = F(2);
#endif
```

### Violation (must be reported): a conditional directive inside macro arguments

`main.c`

```c
#define F(x) (x)
int v = F(
#ifdef A
    1
#else
    2
#endif
);
```

### Compliant (must stay silent): a multi-line macro invocation without directives

`main.c`

```c
#define ADD(a, b) ((a) + (b))
int x = ADD(1,
            2);
```

### Violation (must be reported): #define inside macro arguments

`main.c`

```c
#define F(x) (x)
int a = F(
#define K 1 /*@V*/
    K);
```

### Violation (must be reported): #ifdef inside macro arguments

`main.c`

```c
#define F(x) (x)
int b = F(
#ifdef Q /*@V*/
    1
#else
    2
#endif /*@V*/
    );
```

### Compliant (must stay silent): a directive after the invocation ends

`main.c`

```c
#define F(x) (x)
int a = F(1);
#define K 2
int b = F(K);
```

### Compliant (must stay silent): a hash character inside a string argument

`main.c`

```c
#define F(x) (x)
const char *s = F("#define X");
```

### Compliant (must stay silent): a directive inside a function body, outside any invocation

`main.c`

```c
#define F(x) (x)
int g(void)
{
#if 1
    return F(1);
#else
    return F(2);
#endif
}
```

