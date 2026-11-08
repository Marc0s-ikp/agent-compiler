int f(int *p, int *q)
{
    int d = q - p;
    int e = p - q;
    int *r = q - 2;
    return d * 10000 + e * 100 + *r;
}
