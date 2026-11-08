int f(int x);

int g(int a, int b)
{
    return a * 100 + b;
}

int main()
{
    if (f(3) != 306) return 1;
    return 0;
}
