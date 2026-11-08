int s_char(void)      { return sizeof(char); }
int s_uchar(void)     { return sizeof(unsigned char); }
int s_schar(void)     { return sizeof(signed char); }
int s_int(void)       { return sizeof(int); }
int s_uint(void)      { return sizeof(unsigned); }
int s_uint2(void)     { return sizeof(unsigned int); }
int s_charp(void)     { return sizeof(char *); }
int s_intp(void)      { return sizeof(int *); }
int s_pp(void)        { return sizeof(char **); }
int s_voidp(void)     { return sizeof(void *); }
int s_arr_int(void)   { return sizeof(int[10]); }
int s_arr_char(void)  { return sizeof(char[3]); }
int s_arr2(void)      { return sizeof(int[3][4]); }
int s_arr_ptr(void)   { return sizeof(char *[5]); }
int s_ptr_arr(void)   { return sizeof(int (*)[5]); }
int s_expr_type(void) { return sizeof(char) + sizeof(int) * 10; }
