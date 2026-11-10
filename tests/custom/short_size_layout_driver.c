extern int size_short(void), size_cs(void), size_sc(void), size_si(void), size_ss(void);
extern int size_csc(void), size_union(void), size_array(void), size_expr(void);
extern int off_cs(void), off_sc(void), off_si(void), off_csc(void), align_ptr(void);
extern int ptr_diff(void), ptr_bytes(void), ptr_add(void), ptr_sub(void), ptr_inc(void);
extern int ptr_index(void), ptr_compare(void), two_d(void);

int main(void)
{
    int i, j, s = 0;
    if (size_short() != 2) return 1;
    if (size_cs() != 4 || size_sc() != 4) return 2;
    if (size_si() != 8) return 3;
    if (size_ss() != 6 || size_csc() != 6 || size_union() != 2) return 4;
    if (size_array() != 1002) return 5;
    if (size_expr() != 204) return 6;
    if (off_cs() != 2 || off_sc() != 2 || off_si() != 4) return 7;
    if (off_csc() != 4 * 10 + 2) return 8;
    if (align_ptr() != 0) return 9;
    if (ptr_diff() != 5 || ptr_bytes() != 10) return 10;
    if (ptr_add() != 6 || ptr_sub() != 2 || ptr_inc() != 6) return 11;
    if (ptr_index() != -20 + 30 - 40 - 40) return 12;
    if (!ptr_compare()) return 13;
    for (i = 0; i < 3; i++) for (j = 0; j < 4; j++) s += (i * 4 + j - 6) * (i + 1);
    if (two_d() != s * 1000 + 24 + 18) return 14;
    return 0;
}
