# Rule 5.6

**Category:** Required · **Decidability:** Decidable · **Scope:** System · **Topic:** Identifiers

## What the detector reports

Reports a typedef name that is reused by another declaration (a tag that names the same type is accepted). The finding is reported at the typedef declaration.

Detection contract: Report a typedef whose name is reused by another declaration (a tag naming the same type is accepted).

## Known limits

Typedefs from system headers are not examined.

## Tested cases

6 of 6 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | typedef and struct tag naming the same type | pass |
| Compliant (must stay silent) | unique typedef names | pass |
| Violation (must be reported) | a local variable reuses a typedef name | pass |
| Violation (must be reported) | a file-scope variable named like a typedef in an inner scope function | pass |
| Violation (must be reported) | a parameter named like a typedef | pass |
| Compliant (must stay silent) | the same typedef name for the same type is not repeated | pass |

## Case sources

### Compliant (must stay silent): typedef and struct tag naming the same type

`main.c`

```c
typedef struct node node;
struct node { int v; };
node n;
```

### Compliant (must stay silent): unique typedef names

`main.c`

```c
typedef unsigned int count_t;
typedef int index_t;
count_t c;
index_t i;
```

### Violation (must be reported): a local variable reuses a typedef name

`main.c`

```c
typedef int count_t; /*@V*/
count_t total;
int f(void)
{
    int count_t = 1;
    return count_t;
}
```

### Violation (must be reported): a file-scope variable named like a typedef in an inner scope function

`main.c`

```c
typedef int width_t; /*@V*/
width_t w;
void f(void)
{
    int width_t;
    width_t = 1;
}
```

### Violation (must be reported): a parameter named like a typedef

`main.c`

```c
typedef int id_t2; /*@V*/
int f(int id_t2) { return id_t2; }
```

### Compliant (must stay silent): the same typedef name for the same type is not repeated

`main.c`

```c
typedef unsigned char byte_t;
byte_t b;
```

