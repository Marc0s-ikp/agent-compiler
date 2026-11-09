/* Arguments are converted to the parameter type at the call (prototype). */
double half(double x) { return x / 2.0; }
float halff(float x) { return x / 2.0f; }
int trunc_i(int x) { return x; }
double add3(double a, float b, int c) { return a + b + c; }

double from_int(void) { return half(5); }               /* int -> double */
double from_float(void) { float f; f = 3.0f; return half(f); }
float to_float(void) { return halff(5.0); }             /* double -> float */
float int_to_float(void) { return halff(7); }
int to_int(void) { return trunc_i(7.9); }                /* double -> int */
int float_to_int(void) { float f; f = -2.5f; return trunc_i(f); }
double many(void) { return add3(1, 2, 3) + add3(1.5, 2.5, 3.5); }

/* an argument expression with a nested call and conversions */
double nested(void) { return half(trunc_i(9) + halff(3)); }
double char_arg(void) { char c; c = 100; return half(c); }

/* callee modifies its parameter: a copy */
double clobber(double x) { x = x * 10.0; return x; }
double param_copy(void) { double a; a = 2.0; clobber(a); return a; }

/* taking the address of a floating parameter */
double addr_param(double x)
{
    double *p;
    p = &x;
    *p = *p + 1.0;
    return x;
}
float addr_param_f(float x)
{
    float *p;
    p = &x;
    *p += 2.0f;
    return x;
}
/* a floating parameter past fa7 (so in an integer reg or stack) used by address */
double addr_late(double a, double b, double c, double d, double e,
                 double f, double g, double h, double i, double j)
{
    double *p;
    p = &i;
    *p = *p * 2.0;
    return i + j;
}
