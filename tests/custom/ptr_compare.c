int f(int *a, int i, int j)
{
    int *p = a + i;
    int *q = a + j;
    return (p == q) * 1 + (p != q) * 2 + (p < q) * 4 + (p <= q) * 8 + (p > q) * 16 + (p >= q) * 32;
}
