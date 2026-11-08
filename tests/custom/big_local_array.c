int f(int k)
{
    int before = 11;
    int a[700];
    int after = 22;
    int i;
    for (i = 0; i < 700; i++)
        a[i] = i + k;
    return a[0] + a[699] * 3 + a[350] + *(a + 600) + before + after;
}
