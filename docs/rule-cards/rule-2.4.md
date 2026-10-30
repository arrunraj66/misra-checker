# Rule 2.4

**Category:** Advisory · **Decidability:** Decidable · **Scope:** System · **Topic:** Unused and unreachable code

## What the detector reports

Reports a struct, union or enum tag declared in the main file that is never used as a type.

Detection contract: Report a named struct, union or enum tag declared in the main file that is never referred to as a type.

## Known limits

A tag used only inside a macro that is never expanded is reported. Anonymous types are not tags.

## Tested cases

12 of 12 pass.

| Kind | Case | Result |
|---|---|---|
| Compliant (must stay silent) | tag used for an object | pass |
| Compliant (must stay silent) | tag used through a typedef | pass |
| Compliant (must stay silent) | an anonymous struct has no tag | pass |
| Violation (must be reported) | unused struct tag | pass |
| Violation (must be reported) | unused enum tag | pass |
| Violation (must be reported) | unused union tag | pass |
| Violation (must be reported) | unused tag declared in a function | pass |
| Compliant (must stay silent) | tag completed after a forward declaration and then used | pass |
| Compliant (must stay silent) | enum tag used as a parameter type | pass |
| Compliant (must stay silent) | tag used in sizeof | pass |
| Compliant (must stay silent) | a tag used only through a pointer declaration | pass |
| Compliant (must stay silent) | a tag used in a cast | pass |

## Case sources

### Compliant (must stay silent): tag used for an object

`main.c`

```c
struct point { int x; int y; };
struct point origin;
```

### Compliant (must stay silent): tag used through a typedef

`main.c`

```c
typedef struct node { struct node *next; } node_t;
node_t head;
```

### Compliant (must stay silent): an anonymous struct has no tag

`main.c`

```c
struct { int a; } anon;
```

### Violation (must be reported): unused struct tag

`main.c`

```c
struct unused_tag { int a; }; /*@V*/
```

### Violation (must be reported): unused enum tag

`main.c`

```c
enum unused_enum { A, B }; /*@V*/
```

### Violation (must be reported): unused union tag

`main.c`

```c
union unused_union { int i; float f; }; /*@V*/
```

### Violation (must be reported): unused tag declared in a function

`main.c`

```c
void f(void)
{
    struct local { int a; }; /*@V*/
}
```

### Compliant (must stay silent): tag completed after a forward declaration and then used

`main.c`

```c
struct node;
struct node { struct node *next; };
struct node head;
```

### Compliant (must stay silent): enum tag used as a parameter type

`main.c`

```c
enum mode { M_A, M_B };
int f(enum mode m) { return (int)m; }
```

### Compliant (must stay silent): tag used in sizeof

`main.c`

```c
struct rec { int a; };
unsigned long n = sizeof(struct rec);
```

### Compliant (must stay silent): a tag used only through a pointer declaration

`main.c`

```c
struct opaque;
struct opaque *handle;
```

### Compliant (must stay silent): a tag used in a cast

`main.c`

```c
struct data { int a; };
int f(void *p) { return ((struct data *)p)->a; }
```

