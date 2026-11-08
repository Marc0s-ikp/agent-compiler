int f(int **pp, int *other)
{
    int local = 3;
    int *lp = &local;
    int **q = &lp;
    **q = **q + 1;
    **pp = **pp + 10;
    *pp = other;
    **pp = **pp + 100;
    return local;
}
