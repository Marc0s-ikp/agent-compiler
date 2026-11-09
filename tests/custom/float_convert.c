/* All conversions between int, unsigned, float and double. */
float i2f(int x) { return x; }
double i2d(int x) { return x; }
float u2f(unsigned x) { return x; }
double u2d(unsigned x) { return x; }
int f2i(float x) { return x; }
int d2i(double x) { return x; }
unsigned f2u(float x) { return x; }
unsigned d2u(double x) { return x; }
double f2d(float x) { return x; }
float d2f(double x) { return x; }

/* explicit casts */
int cast_d2i(double x) { return (int)x; }
double cast_i2d(int x) { return (double)x; }
float cast_i2f(int x) { return (float)x; }
double cast_f2d(float x) { return (double)x; }
float cast_d2f(double x) { return (float)x; }
unsigned cast_d2u(double x) { return (unsigned)x; }
double cast_u2d(unsigned x) { return (double)x; }

/* narrow integer types */
int c2d(char c) { double d; d = c; return d * 2.0; }
double sc2d(signed char c) { return c; }
int d2c(double x) { char c; c = x; return c; }
int d2sc(double x) { signed char c; c = x; return c; }
int f2c(float x) { return (char)x; }

/* assignment between types of different kinds */
int assign_conv(void)
{
    float f;
    double d;
    int i;
    f = 7;
    d = f;
    i = d / 2;          /* 3.5 -> 3 */
    f = d / 4;          /* 1.75 */
    return i * 100 + (int)(f * 100.0f);
}

/* int op float promotes the int; the rounding happens in float */
float big_add(void)
{
    float f;
    f = 16777216.0f;
    return f + 1;       /* still 16777216 in float */
}

double big_add_d(void)
{
    float f;
    f = 16777216.0f;
    return f + 1.0;     /* double: 16777217 */
}

/* a negative value to unsigned through int stays defined here */
int neg_trunc(void)
{
    double d;
    d = -2.9;
    return (int)d;      /* -2, toward zero */
}

/* conversions of function results and in the middle of expressions */
int in_expr(int a, double b)
{
    return a * b + 0.5;      /* int * double, double + double, then to int */
}

int cond_conv(int c, int a, double b)
{
    return c ? a : b;        /* common type double, converted back */
}
