float helper(void) { return 2.5f; }

double init_scalars(void)
{
    float a = 1;              /* int to float */
    double b = a;             /* float to double */
    float c = 0.1;            /* double to float */
    double d = 3.5f;
    int e = 2.9;              /* to int */
    double f = a + b * 2;
    return a * 1000.0 + b * 100.0 + d * 10.0 + e + f - c;
}

double init_from_call(void)
{
    double x = helper();
    float y = helper() * 2;
    return x + y;
}

float init_array(void)
{
    float a[4] = {1, 2.5f, 3.5};
    return a[0] + a[1] * 10.0f + a[2] * 100.0f + a[3] * 1000.0f;
}

double init_array_inferred(void)
{
    double a[] = {1.5, 2, 3};
    return sizeof(a) * 100.0 + a[0] + a[1] + a[2];
}

double init_array_expr(double x)
{
    double a[3] = {x, x * 2.0, x + 1};
    return a[0] + a[1] * 10.0 + a[2] * 100.0;
}

double init_2d(void)
{
    float m[2][2] = {{1, 2}, {3}};
    return m[0][0] + m[0][1] * 10.0 + m[1][0] * 100.0 + m[1][1] * 1000.0;
}

double init_uses_earlier(void)
{
    double a = 1.5;
    double b = a * 2.0;
    float c = b + a;
    return c;
}

/* a block-scope static keeps its value */
double static_counter(void)
{
    static double n = 0.5;
    n = n + 1.0;
    return n;
}
