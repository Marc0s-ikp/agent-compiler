int f(int x);

int g(int a, int b, int c, int d, int e, int h, int i, int j)
{
    return a + b * 2 + c * 3 + d * 4 + e * 5 + h * 6 + i * 7 + j * 8;
}

int main()
{
    /* sum of (x+k)*(k+1) for k=0..7 = 36x + 168 */
    if (f(0) != 168) return 1;
    if (f(2) != 240) return 2;
    return 0;
}
