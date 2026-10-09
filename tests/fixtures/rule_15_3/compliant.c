int same_block(int condition)
{
    if (condition != 0)
    {
        goto done;
    }
    return 0;

done:
    return 1;
}

int outer_label(int condition)
{
    int result = 0;
    if (condition != 0)
    {
        if (condition > 5)
        {
            goto finish;
        }
        result = 1;
    }
finish:
    return result;
}
