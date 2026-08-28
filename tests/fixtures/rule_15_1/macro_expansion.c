#define TRANSFER_TO(label_name) goto label_name

int select_value(int condition)
{
    if (condition != 0)
    {
        TRANSFER_TO(selected);
    }
    return 0;

selected:
    return 1;
}
