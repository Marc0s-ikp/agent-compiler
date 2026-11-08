int f(int *p, int i)
{
    int a = *(p + i);
    int b = *(i + p);
    int *q = p + i;
    return a * 10000 + b * 100 + *q;
}
