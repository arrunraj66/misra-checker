#define DECLARE_LABEL(label_name) label_name:
#define JUMP_TO(label_name) goto label_name

int consume_count(int count)
{
    DECLARE_LABEL(again)
    --count;
    if (count > 0)
    {
        JUMP_TO(again);
    }
    return count;
}
