int f(int **pp, int *other);

int main()
{
    int a = 1, b = 2;
    int *p = &a;
    if (f(&p, &b) != 4) return 1;
    if (a != 11) return 2;
    if (b != 102) return 3;
    if (p != &b) return 4;
    return 0;
}
