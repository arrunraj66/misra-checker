static unsigned int f(void)
{
    unsigned int r1 = 0u - 1u;
    unsigned int r2 = 4294967295u + 1u;
    unsigned int r3 = 65536u * 65536u;
    unsigned int r4 = -1u;
    return r1 + r2 + r3 + r4;
}
