float negf(float x) { return -x; }
double negd(double x) { return -x; }
double plusd(double x) { return +x; }
double neg_zero(void) { return -0.0; }
double neg_zero_var(void) { double z; z = 0.0; return -z; }
float neg_neg(float x) { return - -x; }
double neg_expr(double a, double b) { return -(a - b) * 2.0; }

double postinc(void)
{
    double d, old;
    d = 1.5;
    old = d++;
    return old * 10.0 + d;       /* 15 + 2.5 */
}

double preinc(void)
{
    double d, v;
    d = 1.5;
    v = ++d;
    return v * 10.0 + d;         /* 25 + 2.5 */
}

float postdec(void)
{
    float f, old;
    f = 3.0f;
    old = f--;
    return old * 10.0f + f;      /* 30 + 2 */
}

float predec(void)
{
    float f, v;
    f = 0.5f;
    v = --f;
    return v * 10.0f + f;        /* -5 + -0.5 */
}

float incdec_loop(void)
{
    float f;
    int i;
    f = 0.0f;
    for (i = 0; i < 10; i++) f++;
    for (i = 0; i < 4; i++) --f;
    return f;
}

double compound(void)
{
    double d;
    d = 10.0;
    d += 2.5;      /* 12.5 */
    d -= 0.5;      /* 12 */
    d *= 1.5;      /* 18 */
    d /= 4;        /* 4.5, int rhs */
    return d;
}

float compound_f(void)
{
    float f;
    f = 1.0f;
    f += 1;        /* int rhs */
    f *= 3.5;      /* double rhs, result narrowed to float */
    f -= 0.25f;
    return f;      /* 6.75 */
}

/* the lhs is an int: computed as double then truncated */
int compound_int(void)
{
    int i;
    i = 3;
    i += 1.5;      /* 4.5 -> 4 */
    i *= 2.5;      /* 10 */
    i -= 0.5;      /* 9.5 -> 9 */
    return i;
}

/* compound assignment through a pointer, with a side effect in the index */
double compound_ptr(double *p)
{
    int i;
    i = 0;
    p[i++] += 1.0;
    p[i++] *= 2.0;
    return p[0] * 100.0 + p[1] * 10.0 + i;
}

double assign_chain(void)
{
    double a, b;
    float c;
    a = b = c = 2.5;
    return a + b + c;
}

double assign_value(void)
{
    double d;
    int i;
    i = (d = 3.75);      /* value of the assignment has the lhs type */
    return d * 10.0 + i;
}
