/* A6: a homogeneous aggregate of four doubles (32 bytes). By value in v0 to v3 (aarch64; by reference
   on riscv32), then on the stack once the v registers cannot hold it (five doubles first: NSRN 5 + 4 > 8),
   where a later double also goes on the stack although v5 was free (C.3). Results in v0 to v3. Ours calls
   ours and the GCC functions of the driver; the driver calls ours. */
struct D4 { double a, b, c, d; };

double sum_d4(struct D4 s) { return s.a + 2 * s.b + 3 * s.c + 4 * s.d; }

double after_five(double x0, double x1, double x2, double x3, double x4, struct D4 s, double y)
{
    return x0 + 2 * x1 + 3 * x2 + 4 * x3 + 5 * x4 + 100 * s.a + 200 * s.b + 300 * s.c + 400 * s.d + 1000 * y;
}

struct D4 make_d4(double base)
{
    struct D4 t;
    t.a = base; t.b = base + 1; t.c = base + 2; t.d = base + 3;
    return t;
}

double gcc_sum_d4(struct D4 s);
double gcc_after_five(double x0, double x1, double x2, double x3, double x4, struct D4 s, double y);
struct D4 gcc_make_d4(double base);

double ours_to_ours(void)
{
    struct D4 s;
    s = make_d4(1);
    return sum_d4(s) + after_five(1, 2, 3, 4, 5, s, 0.5);
}

double ours_to_gcc(void)
{
    struct D4 s;
    s = gcc_make_d4(1);
    return gcc_sum_d4(s) + gcc_after_five(1, 2, 3, 4, 5, s, 0.5);
}
