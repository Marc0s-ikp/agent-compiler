int f(int x)
{
    int a = 0;
    int b = (a = x, a + 1, a * 2);
    return a * 100 + b;
}
