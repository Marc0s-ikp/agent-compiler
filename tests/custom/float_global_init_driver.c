float get_gf_int(void); double get_gd_int(void); float get_gf(void); double get_gd(void);
float get_gf_from_d(void); double get_gd_from_f(void); int get_gi_trunc(void);
int get_gi_neg(void); int get_gc(void); unsigned get_gu(void); float get_sum(void);
double get_expr(void); double get_cast(void); float get_cast_f(void); int get_cast_i(void);
double get_neg(void); double get_paren(void); int get_cmp(void); double get_sizeof(void);
float get_arr(int i); double get_darr(int i); int darr_size(void);
float get_partial(int i); double get_zero(void); float get_zf(void);
double get_static(void); float get_sarr(int i); double tick(void);
extern float gf_int; extern double gd; extern double counter;

int main()
{
    if (get_gf_int() != 1.0f) return 1;
    if (get_gd_int() != 2.0) return 2;
    if (get_gf() != 1.5f) return 3;
    if (get_gd() != 2.5) return 4;
    if (get_gf_from_d() != 0.25f) return 5;
    if (get_gd_from_f() != 0.5) return 6;
    if (get_gi_trunc() != 2) return 7;
    if (get_gi_neg() != -2) return 8;
    if (get_gc() != 65) return 9;
    if (get_gu() != 3000000000u) return 10;
    if (get_sum() != 3.5f) return 11;
    if (get_expr() != 1.25) return 12;
    if (get_cast() != 3.0) return 13;
    if (get_cast_f() != 3.5f) return 14;
    if (get_cast_i() != 4) return 15;
    if (get_neg() != -2.5) return 16;
    if (get_paren() != 8.0) return 17;
    if (get_cmp() != 1) return 18;
    if (get_sizeof() != 8.0) return 19;
    if (get_arr(0) != 1.0f || get_arr(1) != 2.5f || get_arr(2) != 3.5f) return 20;
    if (get_darr(0) != 1.5 || get_darr(1) != 2.0 || get_darr(2) != 3.0) return 21;
    if (darr_size() != 24) return 22;
    if (get_partial(0) != 1.0f || get_partial(1) != 0.0f || get_partial(3) != 0.0f) return 23;
    if (get_zero() != 0.0 || get_zf() != 0.0f) return 24;
    if (get_static() != 6.25) return 25;
    if (get_sarr(0) != 0.5f || get_sarr(1) != 0.25f) return 26;
    if (gf_int != 1.0f || gd != 2.5) return 27;       /* the symbols are real */
    if (tick() != 1.5 || tick() != 2.5) return 28;
    if (counter != 2.5) return 29;
    return 0;
}
