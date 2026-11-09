int flt(float a, float b) { return a < b; }
int fle(float a, float b) { return a <= b; }
int fgt(float a, float b) { return a > b; }
int fge(float a, float b) { return a >= b; }
int feq(float a, float b) { return a == b; }
int fne(float a, float b) { return a != b; }

int dlt(double a, double b) { return a < b; }
int dle(double a, double b) { return a <= b; }
int dgt(double a, double b) { return a > b; }
int dge(double a, double b) { return a >= b; }
int deq(double a, double b) { return a == b; }
int dne(double a, double b) { return a != b; }

/* int against float, float against double */
int i_lt_f(int i, float f) { return i < f; }
int f_lt_i(float f, int i) { return f < i; }
int d_eq_i(double d, int i) { return d == i; }
int f_eq_d(float f, double d) { return f == d; }
int f_lt_d(float f, double d) { return f < d; }
int u_gt_d(unsigned u, double d) { return u > d; }

/* comparison result feeds arithmetic and conditions */
int sum_cmp(double a, double b)
{
    return (a < b) + (a <= b) * 2 + (a == b) * 4 + (a > b) * 8;
}

int pick(double a, double b)
{
    if (a < b) return 1;
    if (a > b) return 2;
    if (a == b) return 3;
    return 4;                 /* unordered */
}

/* the comparison type is the common type: 0.1f != 0.1 */
int float_vs_double_literal(void)
{
    float f;
    f = 0.1f;
    return f == 0.1;
}

int same_literal(void)
{
    float f;
    f = 0.1;
    return f == 0.1f;
}

int cmp_chain(double x)
{
    return 1.0 < x && x < 2.0;
}
