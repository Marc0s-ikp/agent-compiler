/* Struct moves when the frame is bigger than 2047 bytes: copies, by-reference
   arguments, hidden return pointers and a 3000-byte struct copied by a loop. */
struct R { int a; int b; int c; };
struct S { char pad[3000]; int tail; };
int ours_sum_r(struct R r, int z) { r.a = 100; return r.a + r.b * 10 + r.c * 1000 + z; }
int ours_sum_s(struct S s, int z) { s.pad[0] = 1; return s.pad[2999] + s.pad[5] * 2 + s.tail * 100 + z; }
struct R ours_mk_r(int x) { struct R r; r.a = x; r.b = x + 1; r.c = x + 2; return r; }
struct S ours_mk_s(int x)
{
    struct S s;
    int i;
    for (i = 0; i < 3000; i++) s.pad[i] = 0;
    s.pad[2999] = x; s.pad[5] = x + 1; s.tail = x + 2;
    return s;
}
/* every kind of struct move, in a frame whose offsets do not fit 12 bits */
int ours_frame(int n)
{
    char filler[2500];
    struct R x, y, z;
    struct S s, t;
    int i;
    for (i = 0; i < 2500; i++) filler[i] = i;
    x.a = n; x.b = n + 1; x.c = n + 2;
    y = x;                                   /* small copy, far from s0 */
    z = ours_mk_r(n * 2);                        /* hidden pointer to a far slot */
    s = ours_mk_s(n);                            /* 3004-byte hidden-pointer return */
    t = s;                                   /* copy loop */
    t.tail = t.tail + 1;
    return ours_sum_r(y, filler[2499]) + ours_sum_r(z, 0) + ours_sum_s(t, 0) + ours_sum_s(s, 1)
         + ours_sum_r(ours_mk_r(1), 0) + t.pad[5] + s.tail;
}
