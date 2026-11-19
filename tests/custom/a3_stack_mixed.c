/* A3: narrow integers and pointers in the stack slots after x0-x7 (three stack
   arguments in sum_mixed, so the outgoing area is padded to 16; two in tail_ptr),
   pointer results, and ours calling ours as well as GCC calling ours (the driver). */
int sum_mixed(int a, int b, int c, int d, int e, int f, int g, int h,
              signed char sc, unsigned short us, int *p)
{
    return a + b + c + d + e + f + g + h + sc * 100 + us + *p;
}

int *tail_ptr(int a, int b, int c, int d, int e, int f, int g, int h, int *p, int *q)
{
    if (a == 1) return p;
    return q;
}

int *pick(int *x, int *y, int sel)
{
    if (sel) return x;
    return y;
}

int call_sum(int *p)
{
    return sum_mixed(1, 2, 3, 4, 5, 6, 7, 8, -5, 65535, p);
}

int call_tail(int *p)
{
    return *tail_ptr(1, 2, 3, 4, 5, 6, 7, 8, p, 0) + *pick(p, p, 0);
}
