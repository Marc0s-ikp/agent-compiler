int f(int *a);

int main()
{
    int x[4];
    x[0] = 1; x[1] = 2; x[2] = 3; x[3] = 4;
    if (f(x) != 2000 + 100 + 104 - 1) return 1;
    if (x[3] != 104) return 2;
    return 0;
}
