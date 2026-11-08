int f(int *a, int i);

int main()
{
    int x[4];
    x[0] = 7; x[1] = 8; x[2] = 9; x[3] = 6;
    /* loc[0..2] all equal x[i]; loc[3] = loc[2] */
    if (f(x, 2) != 9000 + 900 + 9) return 1;
    if (f(x, 0) != 7000 + 700 + 7) return 2;
    return 0;
}
