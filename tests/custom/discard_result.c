int bump(int n);

int f(int x)
{
    bump(x);
    bump(x + 1);
    bump(1);
    return 7;
}
