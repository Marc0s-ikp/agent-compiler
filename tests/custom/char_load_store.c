/* Plain char is unsigned (D8): values wrap modulo 256 on store. */
char g;

int store_wrap(int v)
{
    char c;
    c = v;
    return c;
}

int store_neg(void)
{
    char c;
    c = -1;
    return c;
}

int global_roundtrip(int v)
{
    g = v;
    return g;
}

int char_sum(void)
{
    char a;
    char b;
    a = 200;
    b = 100;
    return a + b; /* promoted to int: 300, not wrapped */
}
