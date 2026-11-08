int f(int *a)
{
    int *p = a;
    int r = 0;
    r = r * 10 + *p++;
    r = r * 10 + *p;
    (*p)++;
    r = r * 10 + *p;
    p += 2;
    r = r * 10 + *p;
    p--;
    r = r * 10 + *p;
    p -= 1;
    r = r * 10 + *p;
    --p;
    r = r * 10 + *p;
    ++p;
    return r * 10 + (p - a);
}
