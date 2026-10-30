# Rule 11.1

**Category:** Required · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Pointer conversions

## What the detector reports

Reports a cast that converts to or from a function pointer type, other than the ordinary conversion of a function designator to a pointer.

Detection contract: Report a cast that converts to or from a function pointer type, other than function-designator decay.

## Known limits

A cast to void of a function pointer is not reported (documented gap; the FreeRTOS authors treat it as a violation).

- **False negative:** a cast of a function pointer to void (documented limit)

## Tested cases

8 of 9 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | calling through a function pointer and assigning a compatible one | pass |
| Compliant (must stay silent) | attributes do not change the function pointer type | pass |
| Violation (must be reported) | function pointer cast to void pointer | pass |
| Violation (must be reported) | function pointer cast to an integer | pass |
| Violation (must be reported) | cast between two function pointer types | pass |
| Known false negative (not detected yet) | a cast of a function pointer to void | documented limit |
| Compliant (must stay silent) | casting a null constant to a function pointer type | pass |
| Compliant (must stay silent) | a function designator compared with a function pointer | pass |
| Violation (must be reported) | function pointer cast to unsigned long | pass |

## Case sources

### Compliant (must stay silent): calling through a function pointer and assigning a compatible one

`main.c`

```c
int twice(int x) { return x * 2; }
int (*fp)(int) = twice;
int use(void) { return fp(3); }
```

### Compliant (must stay silent): attributes do not change the function pointer type

`main.c`

```c
typedef void (*task_t)(void *arg);
static void idle(void *arg) __attribute__((noreturn));
static void idle(void *arg) { (void)arg; for (;;) { } }
task_t entry = &idle;
```

### Violation (must be reported): function pointer cast to void pointer

`main.c`

```c
void (*fp)(void);
void *p = (void *)fp; /*@V*/
```

### Violation (must be reported): function pointer cast to an integer

`main.c`

```c
void (*fp)(void);
unsigned long v = (unsigned long)fp; /*@V*/
```

### Violation (must be reported): cast between two function pointer types

`main.c`

```c
int (*a)(int);
long (*b)(long) = (long (*)(long))a; /*@V*/
```

### Known false negative (not detected yet): a cast of a function pointer to void

`main.c`

```c
void (*fp)(void);
void f(void) { (void)fp; } /*@V*/
```

### Compliant (must stay silent): casting a null constant to a function pointer type

`main.c`

```c
void (*fp)(void) = (void (*)(void))0;
```

### Compliant (must stay silent): a function designator compared with a function pointer

`main.c`

```c
void g(void);
int f(void (*fp)(void)) { return fp == g; }
```

### Violation (must be reported): function pointer cast to unsigned long

`main.c`

```c
void g(void);
unsigned long f(void) { return (unsigned long)&g; } /*@V*/
```

