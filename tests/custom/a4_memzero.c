/* A4: zero-filling of local arrays with an initialiser, short (unrolled) and long (loop),
   after the stack has been dirtied by an earlier call. */
void dirty(void)
{
    int junk[200];
    int i;
    for (i = 0; i < 200; i = i + 1) junk[i] = -1;
}

int sum_ints(void)
{
    int a[100] = {7};
    int i, n = 0;
    for (i = 0; i < 100; i = i + 1) n = n + a[i];
    return n;
}

int sum_chars(void)
{
    char c[67] = {5};
    int i, n = 0;
    for (i = 0; i < 67; i = i + 1) n = n + c[i];
    return n;
}

int sum_ptrs(void)
{
    char *p[20] = {0};
    int i, n = 0;
    for (i = 0; i < 20; i = i + 1) n = n + (p[i] != 0);
    return n;
}

int sum_short(void)
{
    char c[13] = {1};
    short s[5] = {2};
    int i, n = 0;
    for (i = 0; i < 13; i = i + 1) n = n + c[i];
    for (i = 0; i < 5; i = i + 1) n = n + s[i];
    return n;
}

int sum_odd_tail(void)
{
    char c[130] = {3};
    int i, n = 0;
    for (i = 0; i < 130; i = i + 1) n = n + c[i];
    return n;
}
