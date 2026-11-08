int f(int x, int y)
{
    int m = x > y ? x : y;
    int s = x ? y ? 1 : 2 : 3;
    return m * 10 + s;
}
