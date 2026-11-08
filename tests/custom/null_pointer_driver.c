int f(int *p);

int main()
{
    int one = 1, zero = 0;
    if (f(0) != 2 + 4) return 1;
    if (f(&one) != 1 + 8 + 16 + 32) return 2;
    if (f(&zero) != 1 + 8 + 32) return 3;
    return 0;
}
