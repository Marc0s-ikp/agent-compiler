char g[2][4] = {"ab", "cd"};

int global_rows(void)
{
    return g[0][0] * 1000000 + g[0][1] * 10000 + g[1][0] * 100 + g[1][1];
}

int global_pad(void)
{
    return g[0][2] + g[0][3] + g[1][2] + g[1][3];
}

int local_rows(void)
{
    char a[2][4] = {"ab", "cd"};
    return a[0][0] * 1000000 + a[0][1] * 10000 + a[1][0] * 100 + a[1][1];
}

void dirty(void)
{
    char junk[64];
    int i;
    for (i = 0; i < 64; i++) junk[i] = 0x55;
}

int local_pad(void)
{
    char a[2][4] = {"ab", "cd"};
    return a[0][2] + a[0][3] + a[1][2] + a[1][3];
}

int local_pad_after_dirty(void)
{
    dirty();
    return local_pad();
}

int sizes(void)
{
    char a[2][4] = {"ab", "cd"};
    return sizeof(a) * 100 + sizeof(a[0]);
}

char *row(int i)
{
    return g[i];
}

int infer_rows(void)
{
    char a[][3] = {"xy", "zw", "uv"};
    return sizeof(a) / sizeof(a[0]);
}
