double l_plain(void); double l_exp(void); double l_exp_cap(void); double l_exp_neg(void);
double l_exp_plus(void); double l_lead_dot(void); double l_trail_dot(void);
double l_lead_dot_exp(void); double l_trail_dot_exp(void); double l_zero(void);
double l_zero_int_exp(void); float l_f(void); float l_F(void); float l_f_exp(void);
float l_f_dot(void); float l_f_trail(void); double l_f_widened(void);
double l_d_precise(void); double l_big(void); double l_small(void); float l_fbig(void);
double l_many_digits(void); double l_leading_zeros(void); double l_digits_after(void);
int is_float(void); int is_double(void); int lit_rounds_to_float(void);
int lit_keeps_double(void); int float_sum_rounds(void); int double_sum_exact(void);
int lit_to_int(void); double lit_neg(void); double lit_in_expr(double x);

int main()
{
    if (l_plain() != 1.5) return 1;
    if (l_exp() != 1000.0) return 2;
    if (l_exp_cap() != 250.0) return 3;
    if (l_exp_neg() != 0.25) return 4;
    if (l_exp_plus() != 15.0) return 5;
    if (l_lead_dot() != 0.5) return 6;
    if (l_trail_dot() != 5.0) return 7;
    if (l_lead_dot_exp() != 2.5) return 8;
    if (l_trail_dot_exp() != 500.0) return 9;
    if (l_zero() != 0.0 || l_zero_int_exp() != 0.0) return 10;
    if (l_f() != 1.5f || l_F() != 1.5f) return 11;
    if (l_f_exp() != 1000.0f) return 12;
    if (l_f_dot() != 0.5f || l_f_trail() != 5.0f) return 13;
    if (l_f_widened() != (double)0.1f) return 14;
    if (l_f_widened() == 0.1) return 15;
    if (l_d_precise() != 0.1) return 16;
    if (l_big() != 1.7976931348623157e308) return 17;
    if (l_small() != 4.9406564584124654e-324) return 18;
    if (l_fbig() != 3.4028234663852886e38f) return 19;
    if (l_many_digits() != 3.14159265358979323846) return 20;
    if (l_leading_zeros() != 7.5) return 21;
    if (l_digits_after() != 1.0) return 22;
    if (!is_float() || !is_double()) return 23;
    if (!lit_rounds_to_float() || !lit_keeps_double()) return 24;
    if (!float_sum_rounds() || !double_sum_exact()) return 25;
    if (lit_to_int() != 3) return 26;
    if (lit_neg() != -150.0) return 27;
    if (lit_in_expr(4.0) != 0.5) return 28;
    return 0;
}
