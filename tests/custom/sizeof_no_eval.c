int counter;

int bump(void)
{
    counter++;
    return 1;
}

int post_inc(void)
{
    int x;
    int s;
    x = 5;
    s = sizeof(x++);
    return s * 100 + x; /* 405 */
}

int assign(void)
{
    int x;
    int s;
    x = 5;
    s = sizeof(x = 9);
    return s * 100 + x; /* 405 */
}

int call(void)
{
    int s;
    counter = 0;
    s = sizeof(bump());
    return s * 100 + counter; /* 400 */
}

int div_zero(void)
{
    int z;
    z = 0;
    return sizeof(1 / z); /* 4, no trap */
}

int deref_null(void)
{
    int *p;
    p = 0;
    return sizeof(*p); /* 4, no fault */
}

int index_oob(void)
{
    int a[2];
    return sizeof(a[1000000]); /* 4 */
}

int logical(void)
{
    int x;
    x = 0;
    return sizeof(x && bump()) * 100 + counter;
}
