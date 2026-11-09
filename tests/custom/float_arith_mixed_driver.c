float addf(float a, float b); float subf(float a, float b); float divf(float a, float b);
double subd(double a, double b); double divd(double a, double b);
double mul_int_d(int i, double d); double d_div_int(double d, int i);
double int_div_d(int i, double d); float f_mul_int(float f, int i);
double f_plus_d(float f, double d); int int_div(int a, int b);
double int_div_then_d(int a, int b); double d_div_then(int a, int b);
double prec(double a, double b, double c); double assoc(double a, double b, double c);
double assoc_div(double a, double b, double c); double paren(double a, double b, double c);
float single_prec(void); double double_prec(void); float float_chain(void);
double widen_chain(void); double tern_mixed(int c); double tern_fd(int c, float f, double d);
float tern_ff(int c, float a, float b); int tern_int_result(int c);
double tern_nested(double x); double comma(void);
double deep(double a, double b, double c, double d);
double int_and_double(int i, double d); double pending(double a, double b);
int loop_sum(void); double accumulate(int n);

int main()
{
    volatile float vf = 0.1f;
    volatile double vd = 0.1;
    float f3 = vf + vf + vf;
    double w = (double)vf + vd;

    if (addf(1.25f, 2.5f) != 3.75f) return 1;
    if (subf(1.25f, 2.5f) != -1.25f) return 2;
    if (divf(1.0f, 4.0f) != 0.25f) return 3;
    if (divf(1.0f, 3.0f) != 1.0f / 3.0f) return 4;
    if (subd(1.25, 2.5) != -1.25) return 5;
    if (divd(1.0, 3.0) != 1.0 / 3.0) return 6;
    if (mul_int_d(3, 1.5) != 4.5) return 7;
    if (d_div_int(7.0, 2) != 3.5) return 8;
    if (int_div_d(7, 2.0) != 3.5) return 9;
    if (f_mul_int(1.5f, 3) != 4.5f) return 10;
    if (f_plus_d(0.5f, 0.25) != 0.75) return 11;
    if (int_div(7, 2) != 3) return 12;
    if (int_div_then_d(7, 2) != 3.0) return 13;
    if (d_div_then(7, 2) != 3.5) return 14;
    if (prec(8.0, 2.0, 3.0) != 8.0 + 2.0 * 3.0 - 8.0 / 2.0) return 15;
    if (assoc(10.0, 3.0, 2.0) != 5.0) return 16;
    if (assoc_div(24.0, 4.0, 2.0) != 3.0) return 17;
    if (paren(1.0, 2.0, 3.0) != 9.0) return 18;
    if (single_prec() != 0.0f) return 19;
    if (double_prec() != 1.0) return 20;
    if (float_chain() != f3) return 21;
    if (widen_chain() != w) return 22;
    if (tern_mixed(1) != 1.0 || tern_mixed(0) != 2.5) return 23;
    if (tern_fd(1, 0.5f, 2.25) != 0.5 || tern_fd(0, 0.5f, 2.25) != 2.25) return 24;
    if (tern_ff(1, 1.5f, 2.5f) != 1.5f || tern_ff(0, 1.5f, 2.5f) != 2.5f) return 25;
    if (tern_int_result(1) != 1 || tern_int_result(0) != 2) return 26;
    if (tern_nested(-3.0) != -1.0 || tern_nested(3.0) != 1.0 || tern_nested(0.0) != 0.0) return 27;
    if (comma() != 4.0) return 28;
    if (deep(1.0, 2.0, 3.0, 6.0) != (1.0 + (2.0 * (3.0 - (6.0 / (1.0 + 2.0))))) * ((1.0 - 2.0) + (3.0 * (6.0 + 1.0)))) return 29;
    if (int_and_double(3, 1.5) != 4.0 * 2.5 + 2.0 * 0.5) return 30;
    if (pending(2.0, 3.0) != 20.0 + 9.0 + 12.5) return 31;
    if (loop_sum() != 20) return 32;
    if (accumulate(4) != 1.0 + 0.5 + 1.0 / 3 + 0.25) return 33;
    return 0;
}
