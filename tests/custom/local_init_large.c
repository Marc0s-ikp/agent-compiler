void dirty(void)
{
    int junk[200];
    int i;
    for (i = 0; i < 200; i++) junk[i] = 0x7f7f7f7f;
}

int big_int(void)
{
    int a[100] = {5, 6};
    int i;
    int s;
    s = 0;
    for (i = 0; i < 100; i++) s += a[i];
    return s * 10 + a[1]; /* 116 */
}

int big_char(void)
{
    char a[101] = "xy";
    int i;
    int s;
    s = 0;
    for (i = 2; i < 101; i++) s += a[i];
    return s + a[0] + a[1];
}

int big_int_dirty(void)
{
    dirty();
    return big_int();
}

int big_char_dirty(void)
{
    dirty();
    return big_char();
}

int unaligned_char_dirty(void)
{
    char pad;
    char a[70] = {1};
    int i;
    int s;
    pad = 9;
    dirty();
    s = 0;
    for (i = 1; i < 70; i++) s += a[i];
    return s + a[0] + pad;
}

int string_in_braces(void)
{
    char s[] = {"hi"};
    return sizeof(s) * 1000 + s[1];
}

int inferred_flat(void)
{
    int a[][2] = {1, 2, 3, 4, 5};
    return sizeof(a) / sizeof(a[0]) * 100 + a[2][0] * 10 + a[2][1];
}

char *ptr_init(void)
{
    char *p = "ptr";
    return p;
}
