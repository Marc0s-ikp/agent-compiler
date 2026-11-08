int walk(char *p)
{
    int n;
    n = 0;
    while (*p) {
        n = n + 1;
        p++;
    }
    return n;
}

int sum_bytes(char *p, int n)
{
    int i;
    int s;
    s = 0;
    for (i = 0; i < n; i++) s += p[i];
    return s;
}

void fill(char *p, int n, int v)
{
    while (n--) *p++ = v;
}

int diff(char *a, char *b)
{
    return b - a;
}

int byte_of_int2(int i)
{
    int v;
    char *p;
    v = 0x04030201;
    p = &v;
    return p[i];
}

int ptr_to_local(void)
{
    char c;
    char *p;
    p = &c;
    *p = 321;
    return c; /* 65 */
}
