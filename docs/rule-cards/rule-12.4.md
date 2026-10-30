# Rule 12.4

**Category:** Advisory · **Decidability:** Decidable · **Scope:** SingleTranslationUnit · **Topic:** Expressions

## What the detector reports

Reports a constant unsigned addition, subtraction, multiplication or shift whose exact result does not fit the type (wrap-around).

Detection contract: Report a constant unsigned addition, subtraction, multiplication or shift whose exact result does not fit the type.

## Known limits

Only constant addition, subtraction and multiplication are evaluated. Shifts are not (whether bits shifted out of an unsigned value count as wrap-around is undecided, and the ~0u << n mask idiom is accepted).

- **False negative:** a left shift that discards bits (whether this is wrap-around is undecided; shifts are not evaluated) (documented limit)

## Tested cases

10 of 11 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | results that fit | pass |
| Violation (must be reported) | unsigned subtraction below zero | pass |
| Violation (must be reported) | unsigned addition past the maximum | pass |
| Violation (must be reported) | unsigned multiplication overflow | pass |
| Violation (must be reported) | complement plus one | pass |
| Violation (must be reported) | multiplication that wraps | pass |
| Compliant (must stay silent) | shift that fits | pass |
| Compliant (must stay silent) | signed constants are another matter | pass |
| Compliant (must stay silent) | a signed expression and a result that fits | pass |
| Compliant (must stay silent) | the common mask idiom that shifts bits out of an all-ones value | pass |
| Known false negative (not detected yet) | a left shift that discards bits (whether this is wrap-around is undecided; shifts are not evaluated) | documented limit |

## Case sources

### Compliant (must stay silent): results that fit

`main.c`

```c
unsigned int a = 5U - 3U;
unsigned int b = 0xFFFFFFFFU - 1U;
unsigned int c = 1U << 31;
```

### Violation (must be reported): unsigned subtraction below zero

`main.c`

```c
unsigned int a = 0U - 1U; /*@V*/
```

### Violation (must be reported): unsigned addition past the maximum

`main.c`

```c
unsigned int b = 0xFFFFFFFFU + 1U; /*@V*/
```

### Violation (must be reported): unsigned multiplication overflow

`main.c`

```c
unsigned int c = 0x10000U * 0x10000U; /*@V*/
```

### Violation (must be reported): complement plus one

`main.c`

```c
unsigned int a = ~0u + 1u; /*@V*/
```

### Violation (must be reported): multiplication that wraps

`main.c`

```c
unsigned int a = 0xFFFFFFFFu * 2u; /*@V*/
```

### Compliant (must stay silent): shift that fits

`main.c`

```c
unsigned int a = 1u << 3;
```

### Compliant (must stay silent): signed constants are another matter

`main.c`

```c
int a = 2147483647 - 1;
```

### Compliant (must stay silent): a signed expression and a result that fits

`main.c`

```c
int a = 2147483646 + 1;
unsigned int b = 4000000000U + 200000000U;
```

### Compliant (must stay silent): the common mask idiom that shifts bits out of an all-ones value

`main.c`

```c
unsigned int mask = ~0u << 6;
```

### Known false negative (not detected yet): a left shift that discards bits (whether this is wrap-around is undecided; shifts are not evaluated)

`main.c`

```c
unsigned int a = 0x80000000U << 1; /*@V*/
```

