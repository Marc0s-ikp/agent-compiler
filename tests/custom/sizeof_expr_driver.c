int s_int_var(void);
int s_char_var(void);
int s_noparen(void);
int s_global(void);
int s_arr(void);
int s_arr_elem(void);
int s_count(void);
int s_garr(void);
int s_deref(void);
int s_deref_ch(void);
int s_ptr(void);
int s_promote(void);
int s_char_lit(void);
int s_add(void);
int s_cmp(void);
int s_neg(void);
int s_addr(void);
int s_2d(void);
int s_param(int a[10]);
int s_sizeof_sizeof(void);
int s_in_expr(void);

int main()
{
    int a[10];
    if (s_int_var() != 4) return 1;
    if (s_char_var() != 1) return 2;
    if (s_noparen() != 4) return 3;
    if (s_global() != 41) return 4;
    if (s_arr() != 40) return 5;
    if (s_arr_elem() != 4) return 6;
    if (s_count() != 10) return 7;
    if (s_garr() != 7) return 8;
    if (s_deref() != 4) return 9;
    if (s_deref_ch() != 1) return 10;
    if (s_ptr() != 4) return 11;
    if (s_promote() != 4) return 12;
    if (s_char_lit() != 4) return 13;
    if (s_add() != 4) return 14;
    if (s_cmp() != 4) return 15;
    if (s_neg() != 4) return 16;
    if (s_addr() != 4) return 17;
    if (s_2d() != 4800 + 160 + 4) return 18;
    if (s_param(a) != 4) return 19;
    if (s_sizeof_sizeof() != 4) return 20;
    if (s_in_expr() != 19) return 21;
    return 0;
}
