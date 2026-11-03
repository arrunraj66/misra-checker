extern int counter;
int counter = 1;
int get(void);
int get(void)
{
    return counter;
}
int main(void)
{
    return get();
}
