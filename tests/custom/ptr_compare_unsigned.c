int f(int *p, int *q)
{
    return (p < q) * 1 + (p > q) * 2 + (p <= q) * 4 + (p >= q) * 8;
}
