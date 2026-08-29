int consume_count(int count)
{
again:
    --count;
    if (count > 0)
    {
        goto again;
    }
    return count;
}
