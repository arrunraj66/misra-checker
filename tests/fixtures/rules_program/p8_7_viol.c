int local_only(void)
{
    return 1;
}
int other_local(void)
{
    return local_only();
}
