int sz_float(void) { return sizeof(float); }
int sz_double(void) { return sizeof(double); }
int sz_arr_f(void) { return sizeof(float[3]); }
int sz_arr_d(void) { return sizeof(double[3]); }
int sz_ptr(void) { return sizeof(float *) + sizeof(double *); }
int sz_lit(void)
{
    return sizeof(1.5) * 1000 + sizeof(1.5f) * 100 + sizeof(1.5f + 1) * 10
         + sizeof(1.5f + 1.0);                         /* 8 4 4 8 */
}
int sz_expr(void)
{
    float f;
    double d;
    return sizeof(f) * 1000 + sizeof(d) * 100 + sizeof(f * f) * 10
         + sizeof(f * d);                              /* 4 8 4 8 */
}
int sz_cmp(void) { return sizeof(1.5 < 2.5); }        /* int */
int sz_cast(void) { return sizeof((float)1.0) * 10 + sizeof((double)1); }
int sz_tern(int c) { return sizeof(c ? 1 : 2.5) * 10 + sizeof(c ? 1.5f : 2); }

/* an unevaluated sizeof operand must have no effect */
int sz_noeval(void)
{
    double d;
    d = 1.0;
    sizeof(d = 5.0);
    return d;
}

float ret_f(void) { return 1.5f; }
double ret_d(void) { return 2.5; }
float ret_f_from_d(void) { return 0.1; }               /* narrowed on return */
double ret_d_from_f(void) { return 0.1f; }             /* widened on return */
double ret_d_from_int(void) { return 3; }
float ret_f_from_int(int x) { return x; }
int ret_i_from_d(void) { return 7.9; }
unsigned ret_u_from_d(void) { return 4000000000.0; }
double ret_d_from_char(void) { char c; c = 200; return c; }
void ret_void_d(double x) { x = x + 1.0; }
double ret_param(double x) { return x; }
float ret_param_f(float x) { return x; }

/* recursion with a double result and a double argument */
double fact(int n) { if (n <= 1) return 1.0; return n * fact(n - 1); }
double power(double b, int n)
{
    if (n == 0) return 1.0;
    return b * power(b, n - 1);
}
double fib_d(double n)
{
    if (n < 2.0) return n;
    return fib_d(n - 1.0) + fib_d(n - 2.0);
}

/* callee-saved fs registers if the compiler uses them: values must survive calls */
double live_across(double a, double b)
{
    double x, y;
    x = ret_param(a) * 2.0;
    y = ret_param(b) * 3.0;
    return x + y + a + b;
}
