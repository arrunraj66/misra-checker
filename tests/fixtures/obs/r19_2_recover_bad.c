/* The undeclared identifier is a compile error; the union after it must still be reported. */
int f(void)
{
    return undeclared_name;
}
union U { int a; float b; };
