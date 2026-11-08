int f(int x, int y)
{
    int gt = x > y;
    int ge = x >= y;
    int ne = x != y;
    return gt * 4 + ge * 2 + ne;
}
