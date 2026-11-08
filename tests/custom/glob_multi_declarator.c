int a, b = 3, c[2], *d = &b, e = 8;

int f()
{
    c[1] = a + 4;
    return *d * 100 + c[1] * 10 + e;
}
