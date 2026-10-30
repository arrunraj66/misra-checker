int two_breaks(int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (i == 5)
        {
            break;
        }
        if (i == 7)
        {
            break;
        }
    }
    return i;
}
