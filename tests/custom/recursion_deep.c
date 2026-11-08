int f(int n)
{
    int a;
    int r;
    if (n == 0)
        return 0;
    a = n * 2;
    r = f(n - 1);
    return r + a - n;
}
