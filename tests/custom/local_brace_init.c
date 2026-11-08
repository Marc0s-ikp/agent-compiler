void dirty(void)
{
    int junk[64];
    int i;
    for (i = 0; i < 64; i++) junk[i] = 0x55555555;
}

int partial(void)
{
    int a[3] = {1, 2};
    return a[0] * 100 + a[1] * 10 + a[2]; /* 120 */
}

int partial_dirty(void)
{
    dirty();
    return partial();
}

int inferred(void)
{
    int b[] = {4, 5, 6};
    return sizeof(b) * 1000 + b[0] * 100 + b[1] * 10 + b[2]; /* 12456 */
}

int exprs(int x)
{
    int c[3] = {x, x + 1, x * 2};
    return c[0] * 100 + c[1] * 10 + c[2];
}

int nested(void)
{
    int m[2][2] = {{1, 2}, {3, 4}};
    return m[0][0] * 1000 + m[0][1] * 100 + m[1][0] * 10 + m[1][1];
}

int nested_partial(void)
{
    int m[2][3] = {{1}, {2, 3}};
    return m[0][0] + m[0][1] + m[0][2] + m[1][0] * 10 + m[1][1] * 100 + m[1][2] * 1000;
}

int nested_partial_dirty(void)
{
    dirty();
    return nested_partial();
}

int scalar_braces(void)
{
    int x = {7};
    return x;
}

int char_list(void)
{
    char c[4] = {'a', 'b'};
    return c[0] + c[1] + c[2] + c[3];
}

int zero_all(void)
{
    int z[8] = {0};
    int i;
    int s;
    s = 0;
    for (i = 0; i < 8; i++) s += z[i];
    return s;
}

int zero_all_dirty(void)
{
    dirty();
    return zero_all();
}

int trailing_comma(void)
{
    int t[3] = {1, 2, 3,};
    return t[0] + t[1] + t[2];
}
