struct Q16 { int a, b, c, d; };
struct Odd11 { char c[11]; };
int one_left(int a, int b, int c, int d, int e, int f, int g, struct Q16 q, int h);
int odd_in_regs(int a, int b, int c, int d, int e, int f, struct Odd11 o, int h);
int ours_to_ours(void);
int ours_to_gcc(void);

int gcc_one_left(int a, int b, int c, int d, int e, int f, int g, struct Q16 q, int h)
{
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 100 * q.a + 200 * q.b + 300 * q.c + 400 * q.d + 10000 * h;
}
int gcc_odd_in_regs(int a, int b, int c, int d, int e, int f, struct Odd11 o, int h)
{
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + o.c[0] * 100 + o.c[10] * 1000 + 10000 * h;
}

int main(void)
{
    struct Q16 q;
    struct Odd11 o;
    int i, want_a, want_b;
    q.a = 1; q.b = 2; q.c = 3; q.d = 4;
    for (i = 0; i < 11; i++) o.c[i] = i + 1;
    want_a = 1 + 4 + 9 + 16 + 25 + 36 + 49 + 100 + 400 + 900 + 1600 + 80000;
    want_b = 1 + 4 + 9 + 16 + 25 + 36 + 100 + 11000 + 90000;
    if (one_left(1, 2, 3, 4, 5, 6, 7, q, 8) != want_a) return 1;
    if (odd_in_regs(1, 2, 3, 4, 5, 6, o, 9) != want_b) return 2;
    if (ours_to_ours() != want_a + want_b) return 3;
    if (ours_to_gcc() != want_a + want_b) return 4;
    return 0;
}
