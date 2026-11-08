int f(int a, int b, int c)
{
    return (a - b - c) * 100000 + (a / b / c) * 1000 + a % b % c + (a << b << c);
}
