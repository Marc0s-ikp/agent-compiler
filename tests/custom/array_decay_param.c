int sum(int *p, int n)
{
    int i;
    int s = 0;
    for (i = 0; i < n; i++)
        s = s + p[i];
    return s;
}

int last(int p[], int n)
{
    return p[n - 1];
}

int f(int n)
{
    int a[10];
    int i;
    for (i = 0; i < 10; i++)
        a[i] = i * i;
    return sum(a, n) * 100 + last(a, n) + sum(a + 2, 3);
}
