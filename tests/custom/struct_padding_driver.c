int sz_a(void); int sz_b(void); int sz_c(void); int sz_d(void); int sz_e(void);
int sz_f(void); int sz_g(void); int sz_h(void); int sz_i(void); int sz_j(void);
int off_a_i(void); int off_b_i(void); int off_d_d(void); int off_f_i(void);
int off_f_d(void); int off_i_i(void); int off_j_d(void); int values(void);

int main()
{
    if (sz_a() != 8) return 1;
    if (sz_b() != 8) return 2;
    if (sz_c() != 8) return 3;
    if (sz_d() != 16) return 4;
    if (sz_e() != 16) return 5;
    if (sz_f() != 16) return 6;
    if (sz_g() != 3) return 7;
    if (sz_h() != 8) return 8;
    if (sz_i() != 12) return 9;
    if (sz_j() != 5) return 10;
    if (off_a_i() != 4) return 11;
    if (off_b_i() != 4) return 12;
    if (off_d_d() != 8) return 13;
    if (off_f_i() != 4) return 14;
    if (off_f_d() != 8) return 15;
    if (off_i_i() != 8) return 16;
    if (off_j_d() != 3) return 17;
    if (values() != 127) return 18;
    return 0;
}
