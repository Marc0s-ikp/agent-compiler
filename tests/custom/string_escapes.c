char *esc(void)
{
    return "a\tb\n\\\"'\101\x42";
}

char *with_nul(void)
{
    return "ab\0cd";
}

int nul_then_more(void)
{
    return "ab\0cd"[3]; /* 'c' */
}

char *quote_chars(void)
{
    return "\"q\"";
}

char *squote(void)
{
    return "'";
}
