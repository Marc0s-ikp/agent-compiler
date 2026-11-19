struct Big { int a[6]; };
struct Big make_big(int a, int b, int c, int d, int e, int f, int g, int h);
int sum_big(int a, int b, int c, int d, int e, int f, int g, int h, struct Big s);
int ours_to_ours(void);
int ours_to_gcc(void);

struct Big gcc_make_big(int a, int b, int c, int d, int e, int f, int g, int h)
{
    struct Big r;
    r.a[0] = a; r.a[1] = b + c; r.a[2] = d; r.a[3] = e + f; r.a[4] = g; r.a[5] = h * 2;
    return r;
}
int gcc_sum_big(int a, int b, int c, int d, int e, int f, int g, int h, struct Big s)
{
    return a + b + c + d + e + f + g + h + 100 * s.a[0] + 200 * s.a[1] + 300 * s.a[5];
}

int main(void)
{
    struct Big r;
    int want;
    r = make_big(1, 2, 3, 4, 5, 6, 7, 8);
    if (r.a[0] != 1 || r.a[1] != 5 || r.a[2] != 4 || r.a[3] != 11 || r.a[4] != 7 || r.a[5] != 16) return 1;
    want = 8 + 100 + 1000 + 4800;
    if (sum_big(1, 1, 1, 1, 1, 1, 1, 1, r) != want) return 2;
    if (ours_to_ours() != want + 5 + 110) return 3;
    if (ours_to_gcc() != want + 5 + 110) return 4;
    return 0;
}
