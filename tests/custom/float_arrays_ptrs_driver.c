extern float fa[4];
extern double da[3];
float sum_f(float *p, int n); double sum_d(double *p, int n);
double sum_arr_param(double a[], int n); void fill_f(float *p, int n);
float global_arrays(void); float local_array(void); int strides(void);
int ptr_diff(void); double align_mix(void); double two_d(double a[2][2]);
void set_through(float *p, double *q); float *first_neg(float *p, int n);
double swap_sum(void);

int main()
{
    float f[4];
    double d[3];
    double m[2][2];
    float g;
    double h;

    f[0] = 1.5f; f[1] = 2.5f; f[2] = 3.0f; f[3] = 4.0f;
    d[0] = 0.5; d[1] = 0.25; d[2] = 8.0;
    if (sum_f(f, 4) != 11.0f) return 1;
    if (sum_d(d, 3) != 8.75) return 2;
    if (sum_arr_param(d, 2) != 0.75) return 3;
    fill_f(f, 4);
    if (f[0] != 0.0f || f[1] != 0.5f || f[3] != 1.5f) return 4;
    if (global_arrays() != 10.0f) return 5;
    if (fa[0] != 1.0f || fa[3] != 4.0f || da[2] != 6.0) return 6;
    if (local_array() != 10.0f) return 7;
    if (strides() != 1224) return 8;
    if (ptr_diff() != 51) return 9;
    if (align_mix() != 6.75) return 10;
    m[0][0] = 1; m[0][1] = 2; m[1][0] = 3; m[1][1] = 4;
    if (two_d(m) != 4321.0) return 11;
    set_through(&g, &h);
    if (g != 1.25f || h != 2.5) return 12;
    f[0] = 1; f[1] = 2; f[2] = -3; f[3] = -4;
    if (first_neg(f, 4) != &f[2]) return 13;
    if (first_neg(f, 2) != 0) return 14;
    if (swap_sum() != 41.5) return 15;
    return 0;
}
