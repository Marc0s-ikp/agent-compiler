int f(int *p)
{
    int *n = 0;
    int r = 0;
    if (p)
        r = r + 1;
    if (!p)
        r = r + 2;
    if (p == 0)
        r = r + 4;
    if (p != n)
        r = r + 8;
    if (p && *p)
        r = r + 16;
    if (p || n)
        r = r + 32;
    return r;
}
