int f(int a, int b, int c)
{
    return a + b * c - (a + b) * c + (a | b & c ^ 1) + (a << 1 + 1);
}
