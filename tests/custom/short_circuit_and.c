int f(int x)
{
    int n = 0;
    int r = x && (n = 5);
    return r * 100 + n;
}
