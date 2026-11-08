int g(int x)
{
    x = x + 100;
    x = x * 2;
    return x;
}

int f(int y)
{
    int r = g(y);
    return r * 1000 + y;
}
