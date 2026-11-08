int f(int x)
{
    int a = x;
    int b = a++;
    int c = ++a;
    int d = a--;
    int e = --a;
    return b * 10000 + c * 1000 + d * 100 + e * 10 + a;
}
