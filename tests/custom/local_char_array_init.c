int len_inferred(void)
{
    char s[] = "abc";
    return sizeof(s); /* 4 */
}

int contents(void)
{
    char s[] = "abc";
    return s[0] * 1000000 + s[1] * 10000 + s[2] * 100 + s[3];
}

int writable(void)
{
    char s[] = "abc";
    s[0] = 'X';
    return s[0] + s[1];
}

int fresh_each_call(void)
{
    char s[] = "ab";
    s[0] = s[0] + 1; /* must start as 'a' on every call */
    return s[0];
}

int empty_str(void)
{
    char s[] = "";
    return sizeof(s) * 10 + s[0];
}

int exact_fit(void)
{
    char s[4] = "abc"; /* no room for NUL in the C sense of [3], here it fits */
    return s[2] + s[3];
}
