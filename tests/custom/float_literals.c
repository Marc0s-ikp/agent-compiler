/* Every spelling of a floating constant that C90 allows (no long double). */
double l_plain(void) { return 1.5; }
double l_exp(void) { return 1e3; }
double l_exp_cap(void) { return 2.5E2; }
double l_exp_neg(void) { return 25e-2; }             /* 0.25 exact */
double l_exp_plus(void) { return 1.5e+1; }
double l_lead_dot(void) { return .5; }
double l_trail_dot(void) { return 5.; }
double l_lead_dot_exp(void) { return .25e1; }
double l_trail_dot_exp(void) { return 5.e2; }
double l_zero(void) { return 0.0; }
double l_zero_int_exp(void) { return 0e0; }
float l_f(void) { return 1.5f; }
float l_F(void) { return 1.5F; }
float l_f_exp(void) { return 1e3f; }
float l_f_dot(void) { return .5f; }
float l_f_trail(void) { return 5.f; }
double l_f_widened(void) { return 0.1f; }
double l_d_precise(void) { return 0.1; }
double l_big(void) { return 1.7976931348623157e308; }
double l_small(void) { return 4.9406564584124654e-324; }
float l_fbig(void) { return 3.4028234663852886e38f; }
double l_many_digits(void) { return 3.14159265358979323846; }
double l_leading_zeros(void) { return 007.5; }       /* decimal, not octal */
double l_digits_after(void) { return 1.000000000000000000001; }

/* the type of the literal decides the arithmetic */
int is_float(void) { return sizeof(1.5f) == 4; }
int is_double(void) { return sizeof(1.5) == 8; }
int lit_rounds_to_float(void)
{
    return 16777217.0f == 16777216.0f;                /* float constant rounds */
}
int lit_keeps_double(void)
{
    return 16777217.0 != 16777216.0;
}
int float_sum_rounds(void)
{
    return (16777216.0f + 1.0f) == 16777216.0f;       /* float arithmetic */
}
int double_sum_exact(void)
{
    return (16777216.0 + 1.0) != 16777216.0;
}

/* a literal used where an int is expected, and a leading unary minus */
int lit_to_int(void) { int i; i = 3.99; return i; }
double lit_neg(void) { return -1.5e2; }
double lit_in_expr(double x) { return x * 2.5 - 1e1 + .5; }
