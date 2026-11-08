int clobber(int a, int b, int c, int d, int e, int g)
{
    int t1 = a * 3;
    int t2 = b * 5 + t1;
    int t3 = c * 7 + t2;
    int t4 = d * 11 + t3;
    int t5 = e * 13 + t4;
    int t6 = g * 17 + t5;
    return t1 + t2 + t3 + t4 + t5 + t6;
}

int f(int a, int b, int c)
{
    int x = a + 1;
    int y = b * 2;
    int z = c - 3;
    int r = clobber(x, y, z, a, b, c);
    int q = clobber(c, b, a, z, y, x);
    return r + q + x + y + z + a + b + c;
}
