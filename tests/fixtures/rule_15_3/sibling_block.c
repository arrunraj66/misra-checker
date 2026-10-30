int sibling_block(int condition)
{
    if (condition != 0)
    {
        goto other;
    }
    else
    {
other:
        condition = 2;
    }
    return condition;
}
