/* Zero, hexadecimal, decimal and floating constants in macro bodies are not octal. */
#define ZERO 0
#define HEXV 0x1F
#define FLT 01.5
#define DEC 10
#define EXPO 0e1
int f(void)
{
    return 1;
}
