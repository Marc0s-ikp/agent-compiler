int sz_float(void); int sz_double(void); int sz_arr_f(void); int sz_arr_d(void);
int sz_ptr(void); int sz_lit(void); int sz_expr(void); int sz_cmp(void);
int sz_cast(void); int sz_tern(int c); int sz_noeval(void);
float ret_f(void); double ret_d(void); float ret_f_from_d(void); double ret_d_from_f(void);
double ret_d_from_int(void); float ret_f_from_int(int x); int ret_i_from_d(void);
unsigned ret_u_from_d(void); double ret_d_from_char(void); void ret_void_d(double x);
double ret_param(double x); float ret_param_f(float x);
double fact(int n); double power(double b, int n); double fib_d(double n);
double live_across(double a, double b);

int main()
{
    if (sz_float() != 4 || sz_double() != 8) return 1;
    if (sz_arr_f() != 12 || sz_arr_d() != 24) return 2;
    if (sz_ptr() != 8) return 3;
    if (sz_lit() != 8448) return 4;
    if (sz_expr() != 4848) return 5;
    if (sz_cmp() != 4) return 6;
    if (sz_cast() != 48) return 7;
    if (sz_tern(1) != 84 || sz_tern(0) != 84) return 8;
    if (sz_noeval() != 1) return 9;
    if (ret_f() != 1.5f || ret_d() != 2.5) return 10;
    if (ret_f_from_d() != 0.1f) return 11;
    if (ret_d_from_f() != (double)0.1f) return 12;
    if (ret_d_from_int() != 3.0) return 13;
    if (ret_f_from_int(-7) != -7.0f) return 14;
    if (ret_i_from_d() != 7) return 15;
    if (ret_u_from_d() != 4000000000u) return 16;
    if (ret_d_from_char() != 200.0) return 17;
    ret_void_d(1.0);
    if (ret_param(1e100) != 1e100) return 18;
    if (ret_param_f(-0.5f) != -0.5f) return 19;
    if (fact(10) != 3628800.0) return 20;
    if (power(1.5, 4) != 5.0625) return 21;
    if (fib_d(10.0) != 55.0) return 22;
    if (live_across(1.0, 2.0) != 2.0 + 6.0 + 3.0) return 23;
    return 0;
}
