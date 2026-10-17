summary: Reports the definition of an externally visible function or object that has no earlier declaration in the translation unit (main is exempt).
limits: A declaration in a header counts as earlier only when it is visible in the same translation unit; whether the declarations are compatible is Rule 8.3.

--- ok: prototype before the definition
int get(void);
int get(void) { return 1; }

--- ok: static functions and objects need no declaration
static int hidden(void) { return 1; }
static int counter;

--- ok: main is exempt
int main(void) { return 0; }

--- ok: object declared extern before its definition
extern int level;
int level = 3;

--- bad: function defined without a prior declaration
int compute(int a) /*@V*/
{
    return a;
}

--- bad: object defined without a prior declaration
int total = 0; /*@V*/

--- bad: a prototype that appears after the definition does not help
int late(void) { return 1; } /*@V*/
int late(void);
