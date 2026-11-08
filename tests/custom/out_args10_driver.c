int f(int x);

int g(int a, int b, int c, int d, int e, int h, int i, int j, int k, int l)
{
    return (a + b + c + d + e + h + i + j) * 1000 + k * 10 + l + (j == 8 ? 0 : 99999);
}

int main()
{
    if (f(3) != 36000 + 30 + 6) return 1;
    if (f(0) != 36000) return 2;
    return 0;
}
