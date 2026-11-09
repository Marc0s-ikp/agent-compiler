float addf(float a, float b) { return a + b; }
float subf(float a, float b) { return a - b; }
float divf(float a, float b) { return a / b; }
double subd(double a, double b) { return a - b; }
double divd(double a, double b) { return a / b; }
double mul_int_d(int i, double d) { return i * d; }
double d_div_int(double d, int i) { return d / i; }
double int_div_d(int i, double d) { return i / d; }
float f_mul_int(float f, int i) { return f * i; }
double f_plus_d(float f, double d) { return f + d; }
int int_div(int a, int b) { return a / b; }
double int_div_then_d(int a, int b) { return a / b; }      /* integer division first */
double d_div_then(int a, int b) { return a / (double)b; }

/* precedence, associativity, parentheses */
double prec(double a, double b, double c) { return a + b * c - a / b; }
double assoc(double a, double b, double c) { return a - b - c; }
double assoc_div(double a, double b, double c) { return a / b / c; }
double paren(double a, double b, double c) { return (a + b) * c; }

/* single versus double precision */
float single_prec(void)
{
    float a, b;
    a = 16777216.0f;
    b = 1.0f;
    return (a + b) - a;           /* 0 in float */
}
double double_prec(void)
{
    double a, b;
    a = 16777216.0;
    b = 1.0;
    return (a + b) - a;           /* 1 */
}
float float_chain(void)
{
    float f;
    f = 0.1f;
    return f + f + f;
}
double widen_chain(void)
{
    float f;
    f = 0.1f;
    return f + 0.1;                /* the float is widened first */
}

/* conditional operator picks the common type */
double tern_mixed(int c) { return c ? 1 : 2.5; }
double tern_fd(int c, float f, double d) { return c ? f : d; }
float tern_ff(int c, float a, float b) { return c ? a : b; }
int tern_int_result(int c) { return c ? 1.5 : 2.5; }
double tern_nested(double x)
{
    return x < 0.0 ? -1.0 : x > 0.0 ? 1.0 : 0.0;
}

/* comma operator and statement expressions with floats */
double comma(void)
{
    double a, b;
    a = 1.5;
    return (b = a * 2.0, b + 1.0);
}

/* many live temporaries: spilling across a nested expression */
double deep(double a, double b, double c, double d)
{
    return (a + (b * (c - (d / (a + b))))) * ((a - b) + (c * (d + 1.0)));
}

/* int and double temporaries live at the same time */
double int_and_double(int i, double d)
{
    return (i + 1) * (d + 1.0) + (i - 1) * (d - 1.0);
}

/* function call result mixed with a pending float temporary */
double sq(double x) { return x * x; }
double pending(double a, double b)
{
    return a * 10.0 + sq(b) + sq(a + b) * 0.5;
}

int loop_sum(void)
{
    double x;
    int n;
    n = 0;
    for (x = 0.0; x < 10.0; x += 0.5) n++;
    return n;
}

double accumulate(int n)
{
    double s;
    int i;
    s = 0;
    for (i = 1; i <= n; i++) s += 1.0 / i;
    return s;
}
