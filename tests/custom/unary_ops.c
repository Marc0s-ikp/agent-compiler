int f(int x)
{
    int a = -x;
    int b = ~x;
    int c = !x;
    int d = +x;
    return a * 1000 + (b & 255) * 4 + c * 2 + (d == x);
}
