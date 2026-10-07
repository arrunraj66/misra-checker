#define CHECK(x) ((void)0)
void f(int a)
{
    CHECK(a);
    (void)a;
}
