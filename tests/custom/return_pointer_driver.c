int f(int *x, int c);

int main()
{
    int a[3];
    a[0] = 0; a[1] = 20; a[2] = 0;
    if (f(a, 1) != 4000 + 21) return 1;
    if (a[0] != 9) return 2;
    a[0] = 0;
    if (f(a, 0) != 900 + 21) return 3;
    if (a[0] != 0) return 4;
    return 0;
}
