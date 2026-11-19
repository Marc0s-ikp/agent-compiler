/* Every expected value is the same expression compiled by GCC with -std=gnu90 (C90 literal types). */
int f_div(void), f_mod(void), f_shr(void), f_shr_hi(void), f_lt(void), f_gt(void), f_dec_max(void);
int f_dec_neg(void), f_long_max(void), f_long_neg(void), f_hex_top(void), f_div_big(void), f_size(void);
int case_lit(int x), arr_mod(void), arr_shr(void);
int r_div(int three), r_mod(int ten), r_shr(int k), r_shr_hi(int k), r_lt(int zero), r_gt(int zero);
int r_dec_max(int seven), r_dec_neg(int zero), r_long_max(int thousand), r_long_neg(int zero);
int r_hex_top(int zero), r_div_big(int one);

#define E_DIV ((int)(0xFFFFFFFFFFFFFFFF / 3 % 1000))
#define E_MOD ((int)(0xFFFFFFFFFFFFFFFF % 10))
#define E_SHR ((int)(0xFFFFFFFFFFFFFFFF >> 63))
#define E_SHR_HI ((int)(0xFFFFFFFFFFFFFFFF >> 33))
#define E_DEC_MAX ((int)(18446744073709551615 / 7 % 1000))
#define E_LONG_MAX ((int)(9223372036854775807 % 1000))

int main(void)
{
    if (f_div() != E_DIV || r_div(3) != E_DIV) return 1;
    if (f_mod() != E_MOD || r_mod(10) != E_MOD) return 2;
    if (f_shr() != E_SHR || r_shr(63) != E_SHR) return 3;
    if (f_shr_hi() != E_SHR_HI || r_shr_hi(33) != E_SHR_HI) return 4;
    if (f_lt() != (0xFFFFFFFFFFFFFFFF < 0) || r_lt(0) != (0xFFFFFFFFFFFFFFFF < 0)) return 5;
    if (f_gt() != (0xFFFFFFFFFFFFFFFF > 0) || r_gt(0) != (0xFFFFFFFFFFFFFFFF > 0)) return 6;
    if (f_dec_max() != E_DEC_MAX || r_dec_max(7) != E_DEC_MAX) return 7;
    if (f_dec_neg() != (-9223372036854775808 < 0) || r_dec_neg(0) != (-9223372036854775808 < 0)) return 8;
    if (f_long_max() != E_LONG_MAX || r_long_max(1000) != E_LONG_MAX) return 9;
    if (f_long_neg() != (-9223372036854775807 < 0) || r_long_neg(0) != (-9223372036854775807 < 0)) return 10;
    if (f_hex_top() != (0x8000000000000000 > 0x7FFFFFFFFFFFFFFF)
        || r_hex_top(0) != (0x8000000000000000 > 0x7FFFFFFFFFFFFFFF)) return 11;
    if (f_div_big() != (0xFFFFFFFFFFFFFFFF / 0x100000000 == 0xFFFFFFFF)
        || r_div_big(1) != (0xFFFFFFFFFFFFFFFF / 0x100000000 == 0xFFFFFFFF)) return 12;
    if (f_size() != (int)(sizeof(0xFFFFFFFFFFFFFFFF) + sizeof(18446744073709551615) * 10
                          + sizeof(9223372036854775807) * 100)) return 13;
    if (case_lit(E_DIV) != 1) return 14;
    if (case_lit(E_MOD + 1000) != 2) return 15;
    if (case_lit(E_SHR + 2000) != 3) return 16;
    if (case_lit((int)(18446744073709551615 % 1000 + 3000)) != 4) return 17;
    if (arr_mod() != (int)(0xFFFFFFFFFFFFFFFF % 1000 + 1)) return 18;
    if (arr_shr() != (int)((0xFFFFFFFFFFFFFFFF >> 58) + 1)) return 19;
    return 0;
}
