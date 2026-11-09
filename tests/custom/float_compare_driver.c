int flt(float a, float b); int fle(float a, float b); int fgt(float a, float b);
int fge(float a, float b); int feq(float a, float b); int fne(float a, float b);
int dlt(double a, double b); int dle(double a, double b); int dgt(double a, double b);
int dge(double a, double b); int deq(double a, double b); int dne(double a, double b);
int i_lt_f(int i, float f); int f_lt_i(float f, int i); int d_eq_i(double d, int i);
int f_eq_d(float f, double d); int f_lt_d(float f, double d); int u_gt_d(unsigned u, double d);
int sum_cmp(double a, double b); int pick(double a, double b);
int float_vs_double_literal(void); int same_literal(void); int cmp_chain(double x);

int main()
{
    volatile double zero = 0.0;
    double nan = zero / zero;
    float nanf = (float)nan;

    if (flt(1.0f, 2.0f) != 1 || flt(2.0f, 2.0f) != 0 || flt(3.0f, 2.0f) != 0) return 1;
    if (fle(1.0f, 2.0f) != 1 || fle(2.0f, 2.0f) != 1 || fle(3.0f, 2.0f) != 0) return 2;
    if (fgt(1.0f, 2.0f) != 0 || fgt(2.0f, 2.0f) != 0 || fgt(3.0f, 2.0f) != 1) return 3;
    if (fge(1.0f, 2.0f) != 0 || fge(2.0f, 2.0f) != 1 || fge(3.0f, 2.0f) != 1) return 4;
    if (feq(2.0f, 2.0f) != 1 || feq(2.0f, 3.0f) != 0) return 5;
    if (fne(2.0f, 2.0f) != 0 || fne(2.0f, 3.0f) != 1) return 6;
    if (flt(-1.0f, 1.0f) != 1 || fgt(-1.0f, -2.0f) != 1) return 7;

    if (dlt(1.0, 2.0) != 1 || dlt(2.0, 2.0) != 0 || dlt(3.0, 2.0) != 0) return 8;
    if (dle(1.0, 2.0) != 1 || dle(2.0, 2.0) != 1 || dle(3.0, 2.0) != 0) return 9;
    if (dgt(1.0, 2.0) != 0 || dgt(2.0, 2.0) != 0 || dgt(3.0, 2.0) != 1) return 10;
    if (dge(1.0, 2.0) != 0 || dge(2.0, 2.0) != 1 || dge(3.0, 2.0) != 1) return 11;
    if (deq(2.0, 2.0) != 1 || deq(2.0, 3.0) != 0) return 12;
    if (dne(2.0, 2.0) != 0 || dne(2.0, 3.0) != 1) return 13;

    if (i_lt_f(2, 2.5f) != 1 || i_lt_f(3, 2.5f) != 0) return 14;
    if (f_lt_i(2.5f, 3) != 1 || f_lt_i(3.5f, 3) != 0) return 15;
    if (d_eq_i(4.0, 4) != 1 || d_eq_i(4.5, 4) != 0) return 16;
    if (f_eq_d(1.5f, 1.5) != 1 || f_eq_d(0.1f, 0.1) != 0) return 17;
    if (f_lt_d(1.0f, 1.5) != 1) return 18;
    if (u_gt_d(4000000000u, 3.0e9) != 1 || u_gt_d(1u, 3.0e9) != 0) return 19;

    if (sum_cmp(1.0, 2.0) != 3) return 20;
    if (sum_cmp(2.0, 2.0) != 6) return 21;
    if (sum_cmp(3.0, 2.0) != 8) return 22;
    if (pick(1.0, 2.0) != 1 || pick(2.0, 1.0) != 2 || pick(1.0, 1.0) != 3) return 23;
    if (float_vs_double_literal() != 0) return 24;
    if (same_literal() != 1) return 25;
    if (cmp_chain(1.5) != 1 || cmp_chain(2.5) != 0 || cmp_chain(0.5) != 0) return 26;

    /* NaN: every ordered compare is false, != is true */
    if (dlt(nan, 1.0) || dle(nan, 1.0) || dgt(nan, 1.0) || dge(nan, 1.0)) return 27;
    if (deq(nan, nan) || !dne(nan, nan)) return 28;
    if (flt(nanf, 1.0f) || fle(nanf, 1.0f) || fgt(nanf, 1.0f) || fge(nanf, 1.0f)) return 29;
    if (feq(nanf, nanf) || !fne(nanf, nanf)) return 30;
    if (pick(nan, 1.0) != 4) return 31;
    if (deq(0.0, -0.0) != 1) return 32;
    return 0;
}
