extern int g_add, g_sub, g_mul_big, g_mul_mid, g_mul_max2, g_neg_min;
extern int g_div_min, g_mod_min, g_shl, g_div_neg, g_mod_neg, g_mod_negd;
int rt_add(int a, int b);
int rt_sub(int a, int b);
int rt_mul(int a, int b);
int rt_div(int a, int b);
int rt_mod(int a, int b);
int rt_shl(int a, int n);
int rt_neg(int a);

#define IMIN (-2147483647 - 1)
#define IMAX 2147483647
/* n: folded value wrong; n+100: run-time value wrong. Expected values are
   literals because the C expressions overflow (undefined in the driver). */
#define CHECK(n, folded, runtime, want) \
    do { if ((folded) != (want)) return n; if ((runtime) != (want)) return n + 100; } while (0)

int main()
{
    CHECK(1, g_add, rt_add(IMAX, 1), IMIN);
    CHECK(2, g_sub, rt_sub(IMIN, 1), IMAX);
    CHECK(3, g_mul_big, rt_mul(65536, 65536), 0);
    CHECK(4, g_mul_mid, rt_mul(100000, 100000), 1410065408);
    CHECK(5, g_mul_max2, rt_mul(IMAX, 2), -2);
    CHECK(6, g_neg_min, rt_neg(IMIN), IMIN);
    CHECK(7, g_div_min, rt_div(IMIN, -1), IMIN);
    CHECK(8, g_mod_min, rt_mod(IMIN, -1), 0);
    CHECK(9, g_shl, rt_shl(1, 31), IMIN);
    CHECK(10, g_div_neg, rt_div(-7, 2), -3);
    CHECK(11, g_mod_neg, rt_mod(-7, 2), -1);
    CHECK(12, g_mod_negd, rt_mod(7, -2), 1);
    return 0;
}
