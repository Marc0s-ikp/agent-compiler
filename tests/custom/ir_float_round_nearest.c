/* int/unsigned/double to float must round to nearest-even, not toward zero. */
float i2f(int i) { return (float)i; }
float u2f_cast(unsigned u) { return (float)u; }
double u2d_exact(unsigned u) { return (double)u; }
float d2f(double d) { return (float)d; }

float assign_i2f(int i)
{
    float f;
    f = i;
    return f;
}

float init_i2f(int i)
{
    float f = i;
    return f;
}

float ret_i2f(int i) { return i; }
float ret_u2f(unsigned u) { return u; }
float ret_d2f(double d) { return d; }

float assign_d2f(double d)
{
    float f;
    f = d;
    return f;
}
