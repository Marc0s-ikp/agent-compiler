int f(int n);

int main()
{
    if (f(0) != 0) return 1;
    if (f(1) != 1) return 2;
    if (f(500) != 125250) return 3;
    return 0;
}
