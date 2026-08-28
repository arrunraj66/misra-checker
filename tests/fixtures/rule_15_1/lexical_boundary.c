static const char message[] = "goto is data, not a statement";

int message_length(void)
{
    /* goto inside a comment must not become an analysis fact. */
    return (int)(sizeof(message) - 1U);
}
