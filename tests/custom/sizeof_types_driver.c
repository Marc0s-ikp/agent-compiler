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
    if (s_char() != sizeof(char)) return 1;
    if (s_uchar() != sizeof(unsigned char)) return 2;
    if (s_schar() != sizeof(signed char)) return 3;
    if (s_int() != sizeof(int)) return 4;
    if (s_uint() != sizeof(unsigned)) return 5;
    if (s_uint2() != sizeof(unsigned int)) return 6;
    if (s_charp() != sizeof(char *)) return 7;
    if (s_intp() != sizeof(int *)) return 8;
    if (s_pp() != sizeof(char **)) return 9;
    if (s_voidp() != sizeof(void *)) return 10;
    if (s_arr_int() != sizeof(int[10])) return 11;
    if (s_arr_char() != sizeof(char[3])) return 12;
    if (s_arr2() != sizeof(int[3][4])) return 13;
    if (s_arr_ptr() != sizeof(char *[5])) return 14;
    if (s_ptr_arr() != sizeof(int (*)[5])) return 15;
    if (s_expr_type() != sizeof(char) + sizeof(int) * 10) return 16;
    return 0;
}
