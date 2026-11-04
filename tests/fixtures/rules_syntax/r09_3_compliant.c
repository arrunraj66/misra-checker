struct holder
{
    int a[3];
};
static const int a1[3] = {1, 2, 3};
static const int a2[3] = {0};
static const int a3[] = {1, 2};
static const char a4[5] = "ab";
static const int a5[4] = {[1] = 5};
static const struct holder h = {{1, 2, 3}};
