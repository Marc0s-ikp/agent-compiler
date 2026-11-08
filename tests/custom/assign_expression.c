int f(int x)
{
    int a;
    int b;
    int c;
    int d;
    a = b = c = x + 1;
    d = (c = 3) + 1;
    return a * 1000 + b * 100 + c * 10 + d;
}
