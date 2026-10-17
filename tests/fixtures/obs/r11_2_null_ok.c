/* Comparing a pointer to an incomplete type with a null pointer constant is allowed. */
#include <stddef.h>
typedef struct Opaque *Handle_t;
Handle_t handle;
int is_set(void)
{
    return handle != NULL;
}
