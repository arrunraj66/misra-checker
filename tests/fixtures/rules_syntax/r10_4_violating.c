#include <stdbool.h>
#include <stdint.h>
enum colour { RED, GREEN };
enum shape { BOX };
static void f(uint32_t u, int32_t s, char c, bool b, enum colour col, enum shape sh)
{
    int32_t r1 = s + u;
    bool r2 = (c == 65);
    uint32_t r3 = u + (-1);
    bool r4 = (b == 1);
    bool r5 = (col == sh);
    int32_t r6 = b ? s : u;
    (void)r1; (void)r2; (void)r3; (void)r4; (void)r5; (void)r6;
}
