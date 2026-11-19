struct D4 { double a, b, c, d; };
double sum_d4(struct D4 s);
double after_five(double x0, double x1, double x2, double x3, double x4, struct D4 s, double y);
struct D4 make_d4(double base);
double ours_to_ours(void);
double ours_to_gcc(void);

double gcc_sum_d4(struct D4 s) { return s.a + 2 * s.b + 3 * s.c + 4 * s.d; }
double gcc_after_five(double x0, double x1, double x2, double x3, double x4, struct D4 s, double y)
{
    return x0 + 2 * x1 + 3 * x2 + 4 * x3 + 5 * x4 + 100 * s.a + 200 * s.b + 300 * s.c + 400 * s.d + 1000 * y;
}
struct D4 gcc_make_d4(double base)
{
    struct D4 t;
    t.a = base; t.b = base + 1; t.c = base + 2; t.d = base + 3;
    return t;
}

int main(void)
{
    struct D4 s, t;
    s.a = 1; s.b = 2; s.c = 3; s.d = 4;
    if (sum_d4(s) != 30) return 1;
    if (after_five(1, 2, 3, 4, 5, s, 0.5) != 55 + 3000 + 500) return 2;
    t = make_d4(10);
    if (t.a != 10 || t.b != 11 || t.c != 12 || t.d != 13) return 3;
    if (ours_to_ours() != gcc_sum_d4(gcc_make_d4(1)) + gcc_after_five(1, 2, 3, 4, 5, gcc_make_d4(1), 0.5)) return 4;
    if (ours_to_gcc() != ours_to_ours()) return 5;
    return 0;
}
