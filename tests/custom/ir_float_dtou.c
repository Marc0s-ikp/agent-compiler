/* double and float to unsigned (above INT_MAX too) and to narrow integers. */
unsigned d2u(double d) { return d; }
unsigned f2u(float f) { return f; }
int d2i(double d) { return d; }
int f2i(float f) { return f; }

unsigned cast_d2u(double d) { return (unsigned)d; }

unsigned local_d2u(void)
{
    double d = 3000000000.0;
    unsigned u = d;
    return u;
}

unsigned char d2uc(double d) { return d; }
signed char d2sc(double d) { return d; }
unsigned short d2us(double d) { return d; }
short d2ss(double d) { return d; }

/* the result of the conversion is used as an unsigned value */
int d2u_gt(double d) { return (unsigned)d > 2147483648u; }
