int s_char(void);
int s_uchar(void);
int s_schar(void);
int s_int(void);
int s_uint(void);
int s_uint2(void);
int s_charp(void);
int s_intp(void);
int s_pp(void);
int s_voidp(void);
int s_arr_int(void);
int s_arr_char(void);
int s_arr2(void);
int s_arr_ptr(void);
int s_ptr_arr(void);
int s_expr_type(void);

int main()
{
    if (s_char() != 1) return 1;
    if (s_uchar() != 1) return 2;
    if (s_schar() != 1) return 3;
    if (s_int() != 4) return 4;
    if (s_uint() != 4) return 5;
    if (s_uint2() != 4) return 6;
    if (s_charp() != 4) return 7;
    if (s_intp() != 4) return 8;
    if (s_pp() != 4) return 9;
    if (s_voidp() != 4) return 10;
    if (s_arr_int() != 40) return 11;
    if (s_arr_char() != 3) return 12;
    if (s_arr2() != 48) return 13;
    if (s_arr_ptr() != 20) return 14;
    if (s_ptr_arr() != 4) return 15;
    if (s_expr_type() != 41) return 16;
    return 0;
}
