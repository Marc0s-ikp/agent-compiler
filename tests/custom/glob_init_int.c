int a = 5;
int b = -3;
int c = 2 + 3 * 4;
int d = (1 << 4) | 3;

int get()
{
    return a * 100 + b * 10 + c;
}

int bump()
{
    a = a + 1;
    return a;
}

int get_d()
{
    return d;
}
