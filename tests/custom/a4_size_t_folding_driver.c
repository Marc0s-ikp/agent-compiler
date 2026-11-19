#include <stddef.h>
/* Every expected value is computed here by GCC from the same expression as the test's. */
int f_div(void), f_mod(void), f_shr_top(void), f_shr_mid(void), f_gt_zero(void), f_lt_zero(void);
int f_ge_one(void), f_le_zero(void), f_div_self(void), f_mod_big(void), f_div_big(void);
int case_div(int x), arr_div(void), arr_shr(void);
int r_div(int five, int three), r_mod(int five, int ten), r_shr_top(int five, int shift);
int r_shr_mid(int five, int three), r_gt_zero(int five, int zero), r_lt_zero(int five, int zero);
int r_ge_one(int five, int one), r_le_zero(int five, int zero), r_div_self(int five);
int r_mod_big(int five, int q), r_div_big(int five, int six);
int r_shl(int five, int n), r_shl_val(int one, int n), r_shr_cmp(int five, int n), r_compound_shr(int five, int n);

#define M (sizeof(int) - 5)
#define E_DIV ((int)(M / 3 % 1000))
#define E_MOD ((int)(M % 10))
#define E_SHR_TOP ((int)(M >> (sizeof(void *) * 8 - 1)))
#define E_SHR_MID ((int)((M >> 3) % 1000))
#define E_SHR_MID_ARR ((int)((M >> (sizeof(void *) * 8 - 2)) + 1))

int main(void)
{
    if (f_div() != E_DIV || r_div(5, 3) != E_DIV) return 1;
    if (f_mod() != E_MOD || r_mod(5, 10) != E_MOD) return 2;
    if (f_shr_top() != E_SHR_TOP || r_shr_top(5, (int)(sizeof(void *) * 8 - 1)) != E_SHR_TOP) return 3;
    if (f_shr_mid() != E_SHR_MID || r_shr_mid(5, 3) != E_SHR_MID) return 4;
    if (f_gt_zero() != (M > 0) || r_gt_zero(5, 0) != (M > 0)) return 5;
    if (f_lt_zero() != (M < 0) || r_lt_zero(5, 0) != (M < 0)) return 6;
    if (f_ge_one() != (M >= 1) || r_ge_one(5, 1) != (M >= 1)) return 7;
    if (f_le_zero() != (M <= 0) || r_le_zero(5, 0) != (M <= 0)) return 8;
    if (f_div_self() != (int)(M / M) || r_div_self(5) != (int)(M / M)) return 9;
    if (f_mod_big() != (int)(M % 1000003) || r_mod_big(5, 1000003) != (int)(M % 1000003)) return 10;
    if (f_div_big() != (int)((sizeof(int) - 6) / M) || r_div_big(5, 6) != (int)((sizeof(int) - 6) / M)) return 11;
    if (case_div(E_DIV) != 1) return 12;
    if (case_div((int)(M % 10 + 2000)) != 2 && E_DIV != (int)(M % 10 + 2000)) return 13;
    if (case_div(E_SHR_TOP) != (E_SHR_TOP == E_DIV ? 1 : 3)) return 14;
    if (arr_div() != (int)(M / 3 % 100 + 1)) return 15;
    if (arr_shr() != E_SHR_MID_ARR) return 16;
    {
        int n = 3, w = (int)(sizeof(void *) * 8 - 1);
        unsigned char c = 200;
        int i = 1000;
        if (r_shl(5, 1) != ((M << 1 >> 1) == M)) return 17;
        if (r_shl_val(1, w) != (int)((sizeof(int) - 4 + 1) << w)) return 18;
        if (r_shr_cmp(5, n) != (M >> n > 0) || r_shr_cmp(5, w) != (M >> w > 0)) return 19;
        c >>= n;
        i <<= n;
        if (r_compound_shr(5, n) != (int)(M >> n) % 1000 + c + i) return 20;
    }
    return 0;
}
