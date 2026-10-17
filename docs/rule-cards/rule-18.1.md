# Rule 18.1

**Category:** Required · **Decidability:** Undecidable · **Scope:** System · **Topic:** Pointers and arrays

## What the detector reports

Subset: reports a constant offset or constant index that falls outside the declared array (a pointer one past the end is accepted).

Detection contract: Subset: report constant offsets or constant indexes that fall outside the declared array.

## Known limits

Only constant offsets on arrays with a known size are examined.

- **False negative:** an index computed through a variable (documented limit)

## Tested cases

4 of 5 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | indexes inside the array and the one-past-the-end pointer | pass |
| Violation (must be reported) | a constant index past the end | pass |
| Violation (must be reported) | a negative constant index | pass |
| Violation (must be reported) | a pointer formed beyond one past the end | pass |
| Known false negative (not detected yet) | an index computed through a variable | documented limit |

## Case sources

### Compliant (must stay silent): indexes inside the array and the one-past-the-end pointer

`main.c`

```c
int a[4];
int first(void) { return a[0]; }
int last(void) { return a[3]; }
int *end = a + 4;
```

### Violation (must be reported): a constant index past the end

`main.c`

```c
int a[4];
int f(void) { return a[4]; } /*@V*/
```

### Violation (must be reported): a negative constant index

`main.c`

```c
int a[4];
int g(void) { return a[-1]; } /*@V*/
```

### Violation (must be reported): a pointer formed beyond one past the end

`main.c`

```c
int a[4];
int *p = a + 5; /*@V*/
```

### Known false negative (not detected yet): an index computed through a variable

`main.c`

```c
int a[4];
int h(void)
{
    int i = 6;
    return a[i]; /*@V*/
}
```

