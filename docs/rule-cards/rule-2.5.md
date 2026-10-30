# Rule 2.5

**Category:** Advisory · **Decidability:** Decidable · **Scope:** System · **Topic:** Unused and unreachable code

## What the detector reports

Reports a macro defined in the main file that is never expanded or tested.

Detection contract: Report a macro defined in the main file that is never expanded or tested.

## Known limits

A macro used only in another translation unit, or only in a header that is not part of this analysis, is reported.

## Tested cases

14 of 14 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | macro that is expanded | pass |
| Compliant (must stay silent) | macro that is only tested with defined | pass |
| Compliant (must stay silent) | macro used by another macro that is used | pass |
| Violation (must be reported) | unused object-like macro | pass |
| Violation (must be reported) | unused function-like macro | pass |
| Violation (must be reported) | macro used only in a skipped region | pass |
| Compliant (must stay silent) | macro redefined after undef and used | pass |
| Violation (must be reported) | function-like macro mentioned only by name | pass |
| Compliant (must stay silent) | macro used as an argument of another macro | pass |
| Compliant (must stay silent) | macro tested with ifndef and defined in the same file | pass |
| Compliant (must stay silent) | stringised macro argument | pass |
| Compliant (must stay silent) | a macro used in an elif | pass |
| Compliant (must stay silent) | a header guard style macro is tested | pass |
| Compliant (must stay silent) | a macro used only inside another macro that is used | pass |

## Case sources

### Compliant (must stay silent): macro that is expanded

`main.c`

```c
#define LIMIT 10
int arr[LIMIT];
```

### Compliant (must stay silent): macro that is only tested with defined

`main.c`

```c
#define FEATURE
#ifdef FEATURE
int on;
#endif
```

### Compliant (must stay silent): macro used by another macro that is used

`main.c`

```c
#define BASE 4
#define TWICE (BASE * 2)
int v = TWICE;
```

### Violation (must be reported): unused object-like macro

`main.c`

```c
#define UNUSED_LIMIT 5 /*@V*/
```

### Violation (must be reported): unused function-like macro

`main.c`

```c
#define SQUARE(x) ((x) * (x)) /*@V*/
```

### Violation (must be reported): macro used only in a skipped region

`main.c`

```c
#define ONLY_SKIPPED 1
#if 0
int a = ONLY_SKIPPED;
#endif
```

### Compliant (must stay silent): macro redefined after undef and used

`main.c`

```c
#define LIMIT 1
#undef LIMIT
#define LIMIT 2
int a = LIMIT;
```

### Violation (must be reported): function-like macro mentioned only by name

`main.c`

```c
#define CALL(x) (x)
int CALL_COUNT;
int f(int CALL2) { return CALL2; }
```

### Compliant (must stay silent): macro used as an argument of another macro

`main.c`

```c
#define ID(x) (x)
#define VAL 5
int a = ID(VAL);
```

### Compliant (must stay silent): macro tested with ifndef and defined in the same file

`main.c`

```c
#define FEATURE
#ifndef FEATURE
int a;
#endif
```

### Compliant (must stay silent): stringised macro argument

`main.c`

```c
#define NAME(x) #x
const char *s = NAME(hello);
```

### Compliant (must stay silent): a macro used in an elif

`main.c`

```c
#define MODE 2
#if MODE == 1
int a;
#elif MODE == 2
int b;
#endif
```

### Compliant (must stay silent): a header guard style macro is tested

`main.c`

```c
#ifndef GUARD_H
#define GUARD_H
int a;
#endif
```

### Compliant (must stay silent): a macro used only inside another macro that is used

`main.c`

```c
#define INNER 3
#define OUTER (INNER + 1)
int a[OUTER];
```

