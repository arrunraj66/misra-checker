int f(int x)
{
    if (x > 0)
        x--;
    else
        x++;
    for (int i = 0; i < 3; ++i)
        x += i;
    while (x > 10)
        x -= 10;
    do
        x++;
    while (x < 5);
    return x;
}
