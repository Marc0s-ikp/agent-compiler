int f(int *p, int *q);

int main()
{
    int x[8];
    int k;
    for (k = 0; k < 8; k++) x[k] = 50 + k;
    if (f(&x[1], &x[6]) != 5 * 10000 - 5 * 100 + 54) return 1;
    if (f(&x[2], &x[2]) != 50) return 2;
    return 0;
}
