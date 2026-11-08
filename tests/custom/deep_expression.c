int f(int a, int b, int c, int d)
{
    return ((a + b) * (c - d)) + ((a - b) * (c + d)) * ((a + c) - (b + d)) + (((a * b) + (c * d)) * ((a + d) * (b + c)));
}
