/* Global float/double initialisers: conversions at compile time. */
float gf_int = 1;
double gd_int = 2;
float gf = 1.5f;
double gd = 2.5;
float gf_from_d = 0.25;           /* double constant stored as float */
double gd_from_f = 0.5f;          /* float constant stored as double */
int gi_trunc = 2.9;               /* truncates toward zero */
int gi_neg = -2.9;
char gc_from_d = 65.9;
unsigned gu_from_d = 3000000000.0;

float g_sum = 1.5f + 2;           /* constant arithmetic with promotion */
double g_expr = 1.0 / 4 + 2 * 0.5;
double g_cast = (double)3;
float g_cast_f = (float)7 / 2;    /* 3.5 */
int g_cast_i = (int)3.99 + 1;
double g_neg = -2.5;
double g_paren = (1.5 + 2.5) * 2;
int g_cmp = 1.5 < 2;              /* integer result of a float compare */
double g_sizeof = sizeof(double); /* int constant into a double */

float g_arr[3] = {1, 2.5, 3.5f};
double g_darr[] = {1.5, 2, 3};    /* size inferred: 3 */
float g_partial[4] = {1.0f};      /* rest zero */
double g_zero;                    /* zero-initialised */
float g_zf;
static double g_static = 6.25;
static float g_sarr[2] = {0.5f, 0.25f};

float get_gf_int(void) { return gf_int; }
double get_gd_int(void) { return gd_int; }
float get_gf(void) { return gf; }
double get_gd(void) { return gd; }
float get_gf_from_d(void) { return gf_from_d; }
double get_gd_from_f(void) { return gd_from_f; }
int get_gi_trunc(void) { return gi_trunc; }
int get_gi_neg(void) { return gi_neg; }
int get_gc(void) { return gc_from_d; }
unsigned get_gu(void) { return gu_from_d; }
float get_sum(void) { return g_sum; }
double get_expr(void) { return g_expr; }
double get_cast(void) { return g_cast; }
float get_cast_f(void) { return g_cast_f; }
int get_cast_i(void) { return g_cast_i; }
double get_neg(void) { return g_neg; }
double get_paren(void) { return g_paren; }
int get_cmp(void) { return g_cmp; }
double get_sizeof(void) { return g_sizeof; }
float get_arr(int i) { return g_arr[i]; }
double get_darr(int i) { return g_darr[i]; }
int darr_size(void) { return sizeof(g_darr); }
float get_partial(int i) { return g_partial[i]; }
double get_zero(void) { return g_zero; }
float get_zf(void) { return g_zf; }
double get_static(void) { return g_static; }
float get_sarr(int i) { return g_sarr[i]; }

/* globals are writable and keep their value between calls */
double counter = 0.5;
double tick(void) { counter = counter + 1.0; return counter; }
