int break_and_goto(int n)
{
    int i = 0;
    do
    {
        if (n == 1)
        {
            break;
        }
        if (n == 2)
        {
            goto out;
        }
        i++;
    } while (i < n);
out:
    return i;
}
