/* '.' on array elements, '->' on pointers that came from array decay, and
   mixing the two on arrays inside structs. */
struct P { int x; int y; };
struct Box { struct P corner[2]; struct P *which; };

int dot_on_element(void)
{
    struct P a[3];
    a[0].x = 1; a[1].x = 2; a[2].x = 3;
    a[1].y = a[0].x + a[2].x;
    return a[1].y * 10 + a[1].x;               /* 42 */
}

int arrow_on_decay(void)
{
    struct P a[3];
    struct P *p;
    p = a;                                      /* decay */
    p->x = 5;
    (p + 1)->x = 6;
    (p + 2)->y = 7;
    return a[0].x * 100 + a[1].x * 10 + a[2].y;     /* 567 */
}

int param_decay_helper(struct P v[], int i) { return v[i].x + (v + i)->y; }
int param_decay(void)
{
    struct P a[2];
    a[0].x = 1; a[0].y = 2; a[1].x = 3; a[1].y = 4;
    return param_decay_helper(a, 0) * 10 + param_decay_helper(a, 1);    /* 30+7 */
}

int star_index_equivalence(void)
{
    struct P a[2];
    a[1].x = 9;
    return (*(a + 1)).x + (a + 1)->x + (&a[1])->x;     /* 27 */
}

int array_in_struct(void)
{
    struct Box b;
    b.corner[0].x = 1; b.corner[0].y = 2; b.corner[1].x = 3; b.corner[1].y = 4;
    b.which = &b.corner[1];
    return b.which->x * 10 + b.which->y + b.corner[0].x * 100;   /* 134 */
}

int array_in_struct_via_ptr(void)
{
    struct Box b;
    struct Box *p;
    p = &b;
    p->corner[1].y = 8;
    p->corner[0].x = 7;
    (p->corner + 1)->x = 6;
    return p->corner[1].y + p->corner[1].x * 10 + p->corner[0].x * 100;   /* 768 */
}

int two_dimensional(void)
{
    struct P g[2][3];
    int i, j, s;
    for (i = 0; i < 2; i++) for (j = 0; j < 3; j++) { g[i][j].x = i; g[i][j].y = j; }
    s = 0;
    for (i = 0; i < 2; i++) for (j = 0; j < 3; j++) s += g[i][j].x * 10 + g[i][j].y;
    return s;                                    /* x: 0*3+1*3 =3 -> 30; y: 2*3=6 -> 36 */
}

int assign_through_decay(void)
{
    struct P a[2];
    struct P *p;
    p = a;
    p[1].x = 4;
    *p = p[1];
    return a[0].x + a[1].x;                       /* 8 */
}
