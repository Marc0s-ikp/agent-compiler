float fa[4];
double da[3];

float sum_f(float *p, int n)
{
    float s;
    int i;
    s = 0.0f;
    for (i = 0; i < n; i++) s += p[i];
    return s;
}

double sum_d(double *p, int n)
{
    double s;
    s = 0.0;
    while (n--) s += *p++;
    return s;
}

double sum_arr_param(double a[], int n)
{
    int i;
    double s;
    s = 0;
    for (i = 0; i < n; i++) s = s + a[i];
    return s;
}

void fill_f(float *p, int n)
{
    int i;
    for (i = 0; i < n; i++) p[i] = i * 0.5f;
}

float global_arrays(void)
{
    int i;
    for (i = 0; i < 4; i++) fa[i] = i + 1;       /* int stored into float */
    for (i = 0; i < 3; i++) da[i] = fa[i] * 2.0;
    return fa[3] + da[2];                        /* 4 + 6 */
}

float local_array(void)
{
    float a[5];
    float *p;
    int i;
    for (i = 0; i < 5; i++) a[i] = i;
    p = a + 2;
    return p[0] + p[1] + p[-1] + *(p + 2);       /* 2 + 3 + 1 + 4 */
}

/* pointer arithmetic strides by sizeof(float) / sizeof(double) */
int strides(void)
{
    float f[4];
    double d[4];
    return (int)((char *)&f[3] - (char *)&f[0]) * 100
         + (int)((char *)&d[3] - (char *)&d[0]);        /* 1200 + 24 */
}

int ptr_diff(void)
{
    double d[8];
    double *a, *b;
    a = &d[1];
    b = &d[6];
    return (b - a) * 10 + (a < b);
}

/* a double at an odd element must stay 8 aligned in a frame with a float */
double align_mix(void)
{
    float f;
    double d;
    char c;
    f = 1.5f;
    c = 3;
    d = 2.25;
    return f + d + c;
}

double two_d(double a[2][2])
{
    return a[0][0] + a[0][1] * 10.0 + a[1][0] * 100.0 + a[1][1] * 1000.0;
}

/* pointer to float write through */
void set_through(float *p, double *q)
{
    *p = 1.25;
    *q = 2.5f;
}

float *first_neg(float *p, int n)
{
    int i;
    for (i = 0; i < n; i++) if (p[i] < 0) return p + i;
    return 0;
}

double swap_sum(void)
{
    double a, b, t;
    double *x, *y;
    a = 1.5; b = 4.0;
    x = &a; y = &b;
    t = *x; *x = *y; *y = t;
    return a * 10.0 + b;                         /* 40 + 1.5 */
}
