int *pick(int *a, int *b, int c)
{
    if (c)
        return a;
    return b;
}

int *at(int *a, int i)
{
    return a + i;
}

int f(int *x, int c)
{
    int y = 40;
    *pick(x, &y, c) = 9;
    *at(x, 2) = *at(x, 1) + 1;
    return *pick(x, &y, 0) * 100 + *at(x, 2);
}
