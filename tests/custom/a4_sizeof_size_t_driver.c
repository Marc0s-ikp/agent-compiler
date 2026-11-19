int s_sizeof_sizeof(void), s_neg_unsigned(void), s_neg_wrap(void), s_sizeof_ptr_diff(void);
int s_size_cmp(void), s_div(void), s_const_neg(void), s_const_sz(void);

int main(void)
{
    int a[4];
    if (s_sizeof_sizeof() != sizeof(sizeof 0)) return 1;
    if (s_neg_unsigned() != (int)(0 - sizeof(int) > 0)) return 2;
    if (s_neg_wrap() != (int)(sizeof(int) - 5 > 0)) return 3;
    if (s_sizeof_ptr_diff() != sizeof(&a[3] - &a[0])) return 4;
    if (s_size_cmp() != (sizeof(char) - sizeof(int) < 1)) return 5;
    if (s_div() != (int)((sizeof(int) - 5) / 3 % 1000)) return 6;
    if (s_const_neg() != (int)(0 - sizeof(int) > 0)) return 7;
    if (s_const_sz() != sizeof(sizeof 0)) return 8;
    return 0;
}
