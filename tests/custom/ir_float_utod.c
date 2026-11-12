/* unsigned to double and float, above INT_MAX too. */
double u2d(unsigned u) { return u; }
float u2f(unsigned u) { return u; }

double local_u2d(void)
{
    unsigned x = 3000000000u;
    double d = x;
    return d;
}

double cast_u2d(unsigned x) { return (double)x / 2; }

double compound_u2d(unsigned x)
{
    double d = 1;
    d += x;
    return d;
}

double ushort_u2d(unsigned short s) { return s; }
double uchar_u2d(unsigned char c) { return c; }
double schar_s2d(signed char c) { return c; }
double int_s2d(int i) { return i; }

/* an unsigned value is not a signed one: compare as double */
int u_above_signed(unsigned x) { return (double)x > 2147483647.0; }
