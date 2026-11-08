int next()
{
    static int n = 0;
    n = n + 1;
    return n;
}

int next_other()
{
    static int n = 100;
    n = n + 1;
    return n;
}

int starts_zero()
{
    static int z;
    static int arr[3];
    z = z + 1;
    arr[1] = arr[1] + 2;
    return z * 10 + arr[1];
}

int recurse(int d)
{
    static int calls;
    calls = calls + 1;
    if (d > 0) recurse(d - 1);
    return calls;
}

int in_loop()
{
    int i;
    int r = 0;
    for (i = 0; i < 3; i++) {
        static int k = 10;
        k = k + 1;
        r = k;
    }
    return r;
}
