float negf(float x); double negd(double x); double plusd(double x);
double neg_zero(void); double neg_zero_var(void); float neg_neg(float x);
double neg_expr(double a, double b); double postinc(void); double preinc(void);
float postdec(void); float predec(void); float incdec_loop(void);
double compound(void); float compound_f(void); int compound_int(void);
double compound_ptr(double *p); double assign_chain(void); double assign_value(void);

int main()
{
    double p[2];
    if (negf(2.5f) != -2.5f || negf(-1.0f) != 1.0f) return 1;
    if (negd(2.5) != -2.5 || negd(-1.0) != 1.0) return 2;
    if (plusd(-4.0) != -4.0) return 3;
    if (1.0 / neg_zero() >= 0.0) return 4;       /* -0.0 has the sign bit */
    if (1.0 / neg_zero_var() >= 0.0) return 5;
    if (1.0 / negd(0.0) >= 0.0) return 6;
    if (1.0 / negd(-0.0) <= 0.0) return 7;       /* negating -0.0 gives +0.0 */
    if (neg_neg(3.0f) != 3.0f) return 8;
    if (neg_expr(1.0, 4.0) != 6.0) return 9;
    if (postinc() != 17.5) return 10;
    if (preinc() != 27.5) return 11;
    if (postdec() != 32.0f) return 12;
    if (predec() != -5.5f) return 13;
    if (incdec_loop() != 6.0f) return 14;
    if (compound() != 4.5) return 15;
    if (compound_f() != 6.75f) return 16;
    if (compound_int() != 9) return 17;
    p[0] = 1.5; p[1] = 2.5;
    if (compound_ptr(p) != 2.5 * 100.0 + 5.0 * 10.0 + 2.0) return 18;
    if (assign_chain() != 7.5) return 19;
    if (assign_value() != 40.5) return 20;
    return 0;
}
