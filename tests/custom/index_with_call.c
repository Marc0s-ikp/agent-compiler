int h(int x)
{
    int pad = x * 2;
    return pad - x + 1;
}

int f(int *a, int i)
{
    a[h(i) + 1] = h(2) + a[h(0)];
    *(a + h(1)) = h(a[0]) * 2;
    return a[h(i) + 1] * 100 + a[h(1)];
}
