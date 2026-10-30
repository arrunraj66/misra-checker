/* uint32_t is unknown here (as when a header that should provide it was not found), so g_ms
 * is an invalid declaration and Clang drops `return g_ms;` from the body. The function is
 * invalid, not a function without a return, so Rule 17.4 must not be reported. */
static volatile uint32_t g_ms = 0U;

static uint32_t millis(void)
{
    return g_ms;
}

uint32_t get_tick(void)
{
    return g_ms;
}
