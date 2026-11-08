int f(int n);

int main()
{
    if (f(0) != 10) return 1;
    if (f(7) != 1) return 2;
    if (f(100) != 10) return 3;
    return 0;
}
