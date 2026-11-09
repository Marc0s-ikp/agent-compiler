double half(double x); float halff(float x); int trunc_i(int x);
double add3(double a, float b, int c);
double from_int(void); double from_float(void); float to_float(void);
float int_to_float(void); int to_int(void); int float_to_int(void);
double many(void); double nested(void); double char_arg(void);
double clobber(double x); double param_copy(void);
double addr_param(double x); float addr_param_f(float x);
double addr_late(double a, double b, double c, double d, double e,
                 double f, double g, double h, double i, double j);

int main()
{
    if (half(5) != 2.5) return 1;               /* GCC converts at this call */
    if (halff(5.0) != 2.5f) return 2;
    if (trunc_i(7.9) != 7) return 3;
    if (add3(1, 2, 3) != 6.0) return 4;
    if (from_int() != 2.5) return 5;
    if (from_float() != 1.5) return 6;
    if (to_float() != 2.5f) return 7;
    if (int_to_float() != 3.5f) return 8;
    if (to_int() != 7) return 9;
    if (float_to_int() != -2) return 10;
    if (many() != 6.0 + 7.0) return 11;
    if (nested() != 5.25) return 12;
    if (char_arg() != 50.0) return 13;
    if (clobber(2.0) != 20.0) return 14;
    if (param_copy() != 2.0) return 15;
    if (addr_param(1.5) != 2.5) return 16;
    if (addr_param_f(1.5f) != 3.5f) return 17;
    if (addr_late(0, 0, 0, 0, 0, 0, 0, 0, 3.0, 4.0) != 10.0) return 18;
    return 0;
}
