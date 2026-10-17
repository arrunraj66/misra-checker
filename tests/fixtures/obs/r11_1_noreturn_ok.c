/* An attribute such as noreturn does not change the function pointer type. */
typedef void (*TaskFunction_t)(void *arg);
static void idle(void *arg) __attribute__((noreturn));
static void idle(void *arg)
{
    (void)arg;
    for (;;) { }
}
TaskFunction_t entry = &idle;
