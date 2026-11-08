int h(int x);
int s9(int a, int b, int c, int d, int e, int g, int i, int j, int k);

int f(int x)
{
    return s9(h(x), h(x + 1), h(x + 2), h(x + 3), h(x + 4), h(x + 5), h(x + 6), h(x + 7), h(x + 8));
}

int h(int x)
{
    int pad1 = x * 3;
    int pad2 = pad1 + 7;
    return x * 10 + pad2 - pad1 - 7;
}

int s9(int a, int b, int c, int d, int e, int g, int i, int j, int k)
{
    return a + b * 2 + c * 3 + d * 4 + e * 5 + g * 6 + i * 7 + j * 8 + k * 9;
}
