char gbuf[4];

int local_fill(void)
{
    char a[5];
    int i;
    int s;
    s = 0;
    for (i = 0; i < 5; i++) a[i] = i * 60; /* 0 60 120 180 240 */
    for (i = 0; i < 5; i++) s += a[i];
    return s; /* 600 */
}

int global_fill(void)
{
    int i;
    for (i = 0; i < 4; i++) gbuf[i] = 250 + i; /* wraps at 256 */
    return gbuf[0] + gbuf[1] * 10 + gbuf[3];
}

int neighbours(void)
{
    char a[3];
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    a[1] = 77;
    return a[0] * 10000 + a[1] * 100 + a[2]; /* 17703 */
}

int sizeof_elems(void)
{
    char a[7];
    return sizeof(a);
}
