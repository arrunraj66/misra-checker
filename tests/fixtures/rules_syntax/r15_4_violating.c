static int count_up(int n)
{
    int i = 0;
    while (i < n)
    {
        if (i == 2)
        {
            break;
        }
        if (i == 5)
        {
            goto done;
        }
        i++;
    }
done:
    return i;
}
