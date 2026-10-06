int into_block(int condition)
{
    if (condition != 0)
    {
        goto inner;
    }
    {
        condition = 3;
inner:
        condition = condition + 1;
    }
    return condition;
}
