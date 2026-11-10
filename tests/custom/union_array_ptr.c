/* Array of unions, pointer to union with ->, pointer arithmetic over unions. */
union U { int i; char c; };
union V { double d; int i; };

int array_sum(void)
{
    union U a[4];
    int i, s;
    for (i = 0; i < 4; i++) a[i].i = i * 10;
    s = 0;
    for (i = 0; i < 4; i++) s += a[i].i;
    return s;                                  /* 60 */
}

int through_pointer(void)
{
    union U u;
    union U *p;
    p = &u;
    p->i = 300;
    p->c = 7;                  /* overwrites the low byte */
    return p->i;               /* 256 + 7 = 263 */
}

int ptr_arith(void)
{
    union V a[3];
    union V *p;
    a[0].i = 1; a[1].i = 2; a[2].i = 3;
    p = a;
    p++;
    return p->i * 100 + (p + 1)->i * 10 + (int)(&a[2] - a);   /* 232 */
}

int stride(void)
{
    union V a[2];
    return (char *)&a[1] - (char *)&a[0];      /* 8 */
}

int set_via_func_helper(union V *p, int n) { p->i = n; return p->i; }
int via_function(void)
{
    union V v;
    return set_via_func_helper(&v, 42) + v.i;   /* 84 */
}
