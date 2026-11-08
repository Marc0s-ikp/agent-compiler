typedef int myint;
typedef myint *pint;
typedef int arr3[3];
typedef pint pair[2];

arr3 gl;

myint twice(myint x)
{
    return x * 2;
}

myint via_pointer(pint p)
{
    *p = *p + 1;
    return *p;
}

int array_type()
{
    arr3 a;
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    gl[2] = a[0] + a[1] + a[2];
    return gl[2];
}

int pointer_array()
{
    myint x;
    myint y;
    pair ps;
    x = 4;
    y = 5;
    ps[0] = &x;
    ps[1] = &y;
    return *ps[0] * 10 + *ps[1];
}

pint ret_ptr(pint p)
{
    return p;
}

int ret_ptr_use()
{
    myint v = 8;
    pint p = ret_ptr(&v);
    return *p;
}
