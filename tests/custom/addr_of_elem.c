int f(int *a)
{
    int *p = &a[3];
    int *q = &*p;
    int *r = &a[1];
    *q = *q + 100;
    return (p - r) * 1000 + (q == p) * 100 + a[3] - (&a[0] == a);
}
