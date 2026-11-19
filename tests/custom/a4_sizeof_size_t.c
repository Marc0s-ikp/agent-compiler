/* A4 (27.5.1): sizeof has the unsigned integer type of pointer width. */
int s_sizeof_sizeof(void) { return sizeof(sizeof 0); }
int s_neg_unsigned(void) { return (int)(0 - sizeof(int) > 0); }
int s_neg_wrap(void) { return (int)(sizeof(int) - 5 > 0); }
int s_sizeof_ptr_diff(void) { int a[4]; return sizeof(&a[3] - &a[0]); }
int s_size_cmp(void) { return sizeof(char) - sizeof(int) < 1; }
int s_div(void) { return (int)((sizeof(int) - 5) / 3 % 1000); }
enum { Const_neg = (int)(0 - sizeof(int) > 0), Const_sz = sizeof(sizeof 0) };
int s_const_neg(void) { return Const_neg; }
int s_const_sz(void) { return Const_sz; }
