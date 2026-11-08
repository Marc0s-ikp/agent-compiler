int g(int x)
{
    int a = x + 1;
    int b = a * 2;
    int c = b - x;
    return a + b + c;
}

int f(int x, int y, int z)
{
    int r1 = x + g(y) * g(z);
    int r2 = g(y) - x;
    int r3 = (x * 3) - g(z) - (y * 5);
    return r1 * 10000 + r2 * 100 + r3;
}
