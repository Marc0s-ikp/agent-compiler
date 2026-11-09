/* Our code calls functions compiled by GCC (defined in the driver). */
float ext_f(float x);
double ext_d(double x);
double ext_mix(int a, double b, float c, int d);
double ext_many(double a, double b, double c, double d, double e,
                double f, double g, double h, double i, double j);
float ext_manyf(float a, float b, float c, float d, float e,
                float f, float g, float h, float i, float j);

float call_f(void)
{
    return ext_f(1.5f) + 1.0f;
}

double call_d(void)
{
    return ext_d(2.5) * 2.0;
}

double call_mix(void)
{
    return ext_mix(1, 2.0, 3.0f, 4);
}

double call_many(void)
{
    return ext_many(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
}

float call_manyf(void)
{
    return ext_manyf(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
}

/* an int expression passed to a double parameter, a double to a float one */
double call_conv(void)
{
    return ext_d(3) + ext_f(2.25);
}

/* call result used in an expression with a live temporary around it */
double call_nested(double x)
{
    return x + ext_d(x * 2.0) * ext_d(1.0);
}
