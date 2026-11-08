int f(int *p, int i);

int main()
{
    int x[6];
    int k;
    for (k = 0; k < 6; k++) x[k] = k * 3 + 1;
    if (f(x, 0) != 1 * 10000 + 1 * 100 + 1) return 1;
    if (f(x, 4) != 13 * 10000 + 13 * 100 + 13) return 2;
    return 0;
}
