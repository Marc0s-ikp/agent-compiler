int size_a(void); int size_b(void); int size_c(void); int size_d(void);
int size_e(void); int size_f(void); int size_var(void); int size_arr(void);

int main()
{
    if (size_a() != 4) return 1;
    if (size_b() != 5) return 2;
    if (size_c() != 8) return 3;
    if (size_d() != 8) return 4;
    if (size_e() != 16) return 5;
    if (size_f() != 4) return 6;
    if (size_var() != 8) return 7;
    if (size_arr() != 312) return 8;
    return 0;
}
