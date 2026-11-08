int g(int a, int b)
{
    return a * 10 + b;
}

int f(int x)
{
    return g(g(1, 2), g(3, x)) + g(g(g(x, x), 0), 1);
}
