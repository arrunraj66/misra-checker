#include <stdlib.h>
static void *my_alloc(unsigned n) { (void)n; return 0; }
void *f(void) { return my_alloc(4U); }
