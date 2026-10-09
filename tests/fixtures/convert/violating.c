int select_value(int condition)
{
    if (condition != 0)
    {
        goto selected;
    }
    return 0;

selected:
    return 1;
}
