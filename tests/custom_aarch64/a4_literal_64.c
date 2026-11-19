/* 27.5.2 and 27.5.5 on LP64: integer literals of 8 bytes, folded and at run time.
   0xFFFFFFFFFFFFFFFF and 18446744073709551615 are unsigned long (C90: a decimal above LONG_MAX
   is unsigned long), 9223372036854775807 is long, 0x8000000000000000 and 9223372036854775808 are
   unsigned long. Each expression is folded (an enum value, a case label, an array size) and also
   computed at run time, where an int parameter keeps the compiler from folding it. The driver
   computes every expected value with GCC (-std=gnu90, a4_literal_64.gccflags) from the same expression. */
enum {
    C_div = 0xFFFFFFFFFFFFFFFF / 3 % 1000,
    C_mod = 0xFFFFFFFFFFFFFFFF % 10,
    C_shr = 0xFFFFFFFFFFFFFFFF >> 63,
    C_shr_hi = (int)(0xFFFFFFFFFFFFFFFF >> 33),
    C_lt = 0xFFFFFFFFFFFFFFFF < 0,
    C_gt = 0xFFFFFFFFFFFFFFFF > 0,
    C_dec_max = 18446744073709551615 / 7 % 1000,
    C_dec_neg = -9223372036854775808 < 0,
    C_long_max = 9223372036854775807 % 1000,
    C_long_neg = -9223372036854775807 < 0,
    C_hex_top = 0x8000000000000000 > 0x7FFFFFFFFFFFFFFF,
    C_div_big = 0xFFFFFFFFFFFFFFFF / 0x100000000 == 0xFFFFFFFF,
    C_size = sizeof(0xFFFFFFFFFFFFFFFF) + sizeof(18446744073709551615) * 10 + sizeof(9223372036854775807) * 100
};

int f_div(void) { return C_div; }
int f_mod(void) { return C_mod; }
int f_shr(void) { return C_shr; }
int f_shr_hi(void) { return C_shr_hi; }
int f_lt(void) { return C_lt; }
int f_gt(void) { return C_gt; }
int f_dec_max(void) { return C_dec_max; }
int f_dec_neg(void) { return C_dec_neg; }
int f_long_max(void) { return C_long_max; }
int f_long_neg(void) { return C_long_neg; }
int f_hex_top(void) { return C_hex_top; }
int f_div_big(void) { return C_div_big; }
int f_size(void) { return C_size; }

/* case labels: each value is distinct, the driver asks for each one */
int case_lit(int x)
{
    switch (x) {
    case 0xFFFFFFFFFFFFFFFF / 3 % 1000: return 1;
    case 0xFFFFFFFFFFFFFFFF % 10 + 1000: return 2;
    case (0xFFFFFFFFFFFFFFFF >> 63) + 2000: return 3;
    case 18446744073709551615 % 1000 + 3000: return 4;
    default: return 0;
    }
}

/* array sizes */
int arr_mod(void) { char a[0xFFFFFFFFFFFFFFFF % 1000 + 1]; return sizeof(a); }
int arr_shr(void) { char a[(0xFFFFFFFFFFFFFFFF >> 58) + 1]; return sizeof(a); }

/* the same at run time: three, ten, k, seven, ... are parameters */
int r_div(int three) { return 0xFFFFFFFFFFFFFFFF / three % 1000; }
int r_mod(int ten) { return 0xFFFFFFFFFFFFFFFF % ten; }
int r_shr(int k) { return 0xFFFFFFFFFFFFFFFF >> k; }
int r_shr_hi(int k) { return (int)(0xFFFFFFFFFFFFFFFF >> k); }
int r_lt(int zero) { return 0xFFFFFFFFFFFFFFFF < zero; }
int r_gt(int zero) { return 0xFFFFFFFFFFFFFFFF > zero; }
int r_dec_max(int seven) { return 18446744073709551615 / seven % 1000; }
int r_dec_neg(int zero) { return -9223372036854775808 < zero; }
int r_long_max(int thousand) { return 9223372036854775807 % thousand; }
int r_long_neg(int zero) { return -9223372036854775807 < zero; }
int r_hex_top(int zero) { return 0x8000000000000000 + zero > 0x7FFFFFFFFFFFFFFF; }
int r_div_big(int one) { return 0xFFFFFFFFFFFFFFFF / (0x100000000 * one) == 0xFFFFFFFF; }
