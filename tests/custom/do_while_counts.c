int f(int x)
{
    int n = 0;
    do {
        n = n + 1;
        x = x - 1;
    } while (x > 0);
    return n;
}
