int f(int a, int b, int c);

static int clobber(int a, int b, int c, int d, int e, int g)
{
    int t1 = a * 3;
    int t2 = b * 5 + t1;
    int t3 = c * 7 + t2;
    int t4 = d * 11 + t3;
    int t5 = e * 13 + t4;
    int t6 = g * 17 + t5;
    return t1 + t2 + t3 + t4 + t5 + t6;
}

int main()
{
    int a = 4, b = 9, c = 20;
    int x = a + 1, y = b * 2, z = c - 3;
    int want = clobber(x, y, z, a, b, c) + clobber(c, b, a, z, y, x) + x + y + z + a + b + c;
    if (f(a, b, c) != want) return 1;
    return 0;
}
