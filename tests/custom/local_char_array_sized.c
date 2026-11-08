void dirty(void)
{
    char junk[64];
    int i;
    for (i = 0; i < 64; i++) junk[i] = 0x55;
}

int zero_fill(void)
{
    char s[10] = "abc";
    int i;
    int sum;
    sum = 0;
    for (i = 3; i < 10; i++) sum += s[i];
    return sum; /* 0 */
}

int head(void)
{
    char s[10] = "abc";
    return s[0] + s[1] + s[2];
}

int size(void)
{
    char s[10] = "abc";
    return sizeof(s);
}

int run(void)
{
    dirty();
    return zero_fill();
}
