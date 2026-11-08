int f(int x, int y, int z);

static int g(int x)
{
    int a = x + 1;
    int b = a * 2;
    int c = b - x;
    return a + b + c;
}

int main()
{
    int x = 7, y = 3, z = 5;
    int r1 = x + g(y) * g(z);
    int r2 = g(y) - x;
    int r3 = (x * 3) - g(z) - (y * 5);
    if (f(x, y, z) != r1 * 10000 + r2 * 100 + r3) return 1;
    if (r1 != 7 + 17 * 25) return 2;
    return 0;
}
