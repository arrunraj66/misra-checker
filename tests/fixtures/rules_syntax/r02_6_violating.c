static void f(int x)
{
    if (x > 0)
    {
        goto used;
    }
used:
    x--;
unused:
    x++;
}
