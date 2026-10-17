# Rule 2.3

**Category:** Advisory · **Decidability:** Decidable · **Scope:** System · **Topic:** Unused and unreachable code

## What the detector reports

Reports a typedef declared in the main file that no type in the translation unit refers to.

Detection contract: Report a typedef declared in the main file that no type in the translation unit refers to.

## Known limits

Only typedefs declared in the main file are examined, and a typedef used only in another translation unit is reported here.

## Tested cases

8 of 8 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | typedef that is used | pass |
| Compliant (must stay silent) | typedef used only to build another typedef that is used | pass |
| Violation (must be reported) | unused typedef | pass |
| Violation (must be reported) | unused function-pointer typedef | pass |
| Violation (must be reported) | typedef whose only user is also unused (the outer one is reported) | pass |
| Compliant (must stay silent) | typedef used only inside sizeof | pass |
| Compliant (must stay silent) | typedef used in a cast | pass |
| Compliant (must stay silent) | typedef used as a parameter type of a prototype | pass |

## Case sources

### Compliant (must stay silent): typedef that is used

`main.c`

```c
typedef unsigned int count_t;
count_t total;
```

### Compliant (must stay silent): typedef used only to build another typedef that is used

`main.c`

```c
typedef int base_t;
typedef base_t derived_t;
derived_t v;
```

### Violation (must be reported): unused typedef

`main.c`

```c
typedef int unused_t; /*@V*/
```

### Violation (must be reported): unused function-pointer typedef

`main.c`

```c
typedef void (*callback_t)(int); /*@V*/
```

### Violation (must be reported): typedef whose only user is also unused (the outer one is reported)

`main.c`

```c
typedef int inner_t;
typedef inner_t outer_t; /*@V*/
```

### Compliant (must stay silent): typedef used only inside sizeof

`main.c`

```c
typedef int word_t;
unsigned long n = sizeof(word_t);
```

### Compliant (must stay silent): typedef used in a cast

`main.c`

```c
typedef unsigned char byte_t;
int f(int x) { return (byte_t)x; }
```

### Compliant (must stay silent): typedef used as a parameter type of a prototype

`main.c`

```c
typedef int id_t2;
void use(id_t2 a);
```

