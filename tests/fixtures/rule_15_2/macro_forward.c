#define JUMP_TO(label_name) goto label_name

int select_value(int condition)
{
    if (condition != 0)
    {
        JUMP_TO(selected);
    }
    return 0;

selected:
    return 1;
}
