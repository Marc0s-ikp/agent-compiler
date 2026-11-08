int f(int x);

int count = 0;

int bump(int n)
{
    count += n;
    return n * 99;
}

int main()
{
    if (f(3) != 7) return 1;
    if (count != 3 + 4 + 1) return 2;
    return 0;
}
