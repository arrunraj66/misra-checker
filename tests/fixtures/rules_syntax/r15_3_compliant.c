static void f(int x)
{
    if (x > 0)
    {
        goto out;
    }
    x++;
out:
    x--;
    {
        goto done;
    }
done:
    x++;
}
