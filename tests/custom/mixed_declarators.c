int f(int v)
{
    int *p, q;
    int a[3], b;
    int *ap[3];
    int x = 1, y = 2, z = 3;
    q = v;
    p = &q;
    b = 5;
    a[0] = 10; a[1] = 20; a[2] = 30;
    ap[0] = &x; ap[1] = &y; ap[2] = &z;
    *ap[1] = *ap[1] + a[2];
    **(ap + 2) = 7;
    return *p + q + a[1] + b + y * 1000 + z * 100000 + *ap[0] * 10000000;
}
