void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

void add_into(int *dst, int v)
{
    *dst = *dst + v;
}

int f(int x, int y)
{
    swap(&x, &y);
    add_into(&x, 100);
    add_into(&y, x);
    return x * 1000 + y;
}
