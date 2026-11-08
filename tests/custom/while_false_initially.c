int f(int x)
{
    int n = 0;
    while (x > 100) {
        n = n + 1;
        x = x - 1;
    }
    return n * 1000 + x;
}
