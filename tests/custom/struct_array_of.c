/* Arrays of structs: indexing, nested member arrays, struct elements copied
   as values, global and local arrays. */
struct P { int x; int y; };
struct Rec { char name[4]; struct P pos; };

struct P g_pts[3];
struct Rec g_recs[2];

int local_array(void)
{
    struct P a[4];
    int i, s;
    for (i = 0; i < 4; i++) { a[i].x = i; a[i].y = i * i; }
    s = 0;
    for (i = 0; i < 4; i++) s += a[i].x * 10 + a[i].y;
    return s;                            /* (0+0)+(10+1)+(20+4)+(30+9) = 74 */
}

int global_array(void)
{
    int i;
    for (i = 0; i < 3; i++) { g_pts[i].x = i + 1; g_pts[i].y = (i + 1) * 10; }
    return g_pts[0].x + g_pts[1].y + g_pts[2].x + g_pts[2].y;   /* 1+20+3+30 */
}

int nested_member_array(void)
{
    g_recs[1].name[0] = 'h'; g_recs[1].name[1] = 'i'; g_recs[1].name[2] = 0;
    g_recs[1].pos.x = 5; g_recs[1].pos.y = 6;
    g_recs[0].name[0] = 'z';
    return g_recs[1].name[1] + g_recs[1].pos.y * 1000 + (g_recs[0].name[0] == 'z') * 100000;
    /* 'i' = 105; 6000; 100000 */
}

int element_copy(void)
{
    struct P a[2];
    struct P t;
    a[0].x = 1; a[0].y = 2; a[1].x = 3; a[1].y = 4;
    t = a[0];
    a[0] = a[1];
    a[1] = t;
    return a[0].x * 1000 + a[0].y * 100 + a[1].x * 10 + a[1].y;   /* 3412 */
}

int computed_index(void)
{
    struct P a[3];
    int i;
    a[0].x = 7; a[1].x = 8; a[2].x = 9;
    i = 1;
    return a[i + 1].x * 100 + a[i].x * 10 + a[i - 1].x;    /* 987 */
}

int size_and_stride(void)
{
    struct Rec r[2];
    return ((char *)&r[1] - (char *)&r[0]) * 10 + sizeof(r) / sizeof(r[0]);  /* 12*10 + 2 */
}
