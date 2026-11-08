char *joined(void)
{
    return "ab" "cd" "e";
}

char *joined_lines(void)
{
    return "one "
           "two "
           "three";
}

int joined_len(void)
{
    char *p;
    int n;
    p = "ab" "" "cd";
    n = 0;
    while (p[n]) n++;
    return n;
}

int joined_escape(void)
{
    return "a\n" "b"[2]; /* [] binds to the whole concatenated literal */
}
