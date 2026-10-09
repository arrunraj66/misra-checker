int one_break(int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (i == 5)
        {
            break;
        }
    }
    return i;
}

int switch_in_loop(int n)
{
    int total = 0;
    while (n > 0)
    {
        switch (n)
        {
            case 1:
                total += 1;
                break;
            case 2:
                total += 2;
                break;
            default:
                break;
        }
        n--;
    }
    return total;
}

int inner_goto_stays(int n)
{
    while (n > 0)
    {
        if (n == 3)
        {
            goto next;
        }
        n--;
next:
        n--;
    }
    return n;
}
