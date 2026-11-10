/* Pointers to structs: -> access, address-of members, pointer arithmetic
   scaled by sizeof(struct), pointer difference, writes through pointers. */
struct P { int x; char c; double d; };      /* 16 */

int arrow(void)
{
    struct P p;
    struct P *q;
    q = &p;
    q->x = 4; q->c = 5; q->d = 6.0;
    return p.x * 100 + p.c * 10 + (int)p.d;           /* 456 */
}

int deref_dot(void)
{
    struct P p;
    struct P *q;
    q = &p;
    (*q).x = 7;
    return p.x + (*q).x;                              /* 14 */
}

int stride(void)
{
    struct P a[4];
    struct P *q;
    q = a;
    return (char *)(q + 3) - (char *)q;               /* 48 */
}

int walk(void)
{
    struct P a[4];
    struct P *q;
    int s;
    for (q = a; q < a + 4; q++) q->x = (int)(q - a) + 1;
    s = 0;
    for (q = a; q != a + 4; q++) s = s * 10 + q->x;
    return s;                                          /* 1234 */
}

int diff(void)
{
    struct P a[8];
    return &a[7] - &a[2];                              /* 5 */
}

int increment_deref(void)
{
    struct P a[2];
    struct P *q;
    a[0].x = 1; a[1].x = 2;
    q = a;
    q++;
    q->x++;
    q->x += 10;
    return a[1].x * 10 + a[0].x;                       /* 13*10+1 */
}

int member_address(void)
{
    struct P p;
    int *ip;
    char *cp;
    double *dp;
    ip = &p.x; cp = &p.c; dp = &p.d;
    *ip = 1; *cp = 2; *dp = 3.0;
    return p.x * 100 + p.c * 10 + (int)p.d;           /* 123 */
}

void set(struct P *p, int v) { p->x = v; p->c = v + 1; p->d = v + 2; }
int via_function(void)
{
    struct P p;
    set(&p, 10);
    return p.x + p.c + (int)p.d;                      /* 10+11+12 */
}

int pointer_to_pointer(void)
{
    struct P p;
    struct P *q;
    struct P **qq;
    q = &p; qq = &q;
    (*qq)->x = 9;
    (**qq).c = 8;
    return p.x * 10 + p.c;                            /* 98 */
}
