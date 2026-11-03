#include <stddef.h>
struct hidden;
static struct hidden *same(struct hidden *p, void *v)
{
    struct hidden *a = (struct hidden *)p;
    struct hidden *n = NULL;
    void *w = (void *)p;
    (void)v; (void)n; (void)w;
    return a;
}
