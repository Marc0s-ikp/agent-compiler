int f(int *a, int i, int j);

int main()
{
    int x[5];
    if (f(x, 1, 3) != 2 + 4 + 8) return 1;
    if (f(x, 3, 1) != 2 + 16 + 32) return 2;
    if (f(x, 2, 2) != 1 + 8 + 32) return 3;
    return 0;
}
