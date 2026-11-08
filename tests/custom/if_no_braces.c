int f(int x)
{
    int r = 5;
    if (x > 0)
        r = 10;
    if (x < 0)
        r = 20;
    else if (x == 0)
        r = 30;
    else
        r = 40;
    return r;
}
