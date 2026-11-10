extern int g_and, g_or_neg, g_xor, g_not_zero, g_not_max, g_neg, g_neg_shl;
extern int g_lnot_five, g_lnot_zero, g_lnot_umax, g_lnot_wrap, g_lnot_uwrap, g_plus;
extern unsigned g_and_u, g_or_u, g_xor_u, g_xor_mix, g_not_u, g_not_wrap;
extern unsigned g_neg_u, g_neg_u2, g_plus_u;
unsigned rt_and(int a, unsigned b);
int rt_and_i(int a, int b);
int rt_or_i(int a, int b);
unsigned rt_or_u(unsigned a, int b);
unsigned rt_xor_u(unsigned a, unsigned b);
int rt_xor_i(int a, int b);
int rt_not_i(int a);
unsigned rt_not_u(unsigned a);
unsigned rt_neg_u(unsigned a);
int rt_neg_i(int a);
int rt_lnot_i(int a);
int rt_lnot_u(unsigned a);
unsigned rt_plus_u(unsigned a);
int rt_plus_i(int a);

/* n: folded value wrong; n+100: run-time value wrong. */
#define CHECK(n, folded, runtime, want) \
    do { if ((folded) != (want)) return n; if ((runtime) != (want)) return n + 100; } while (0)

int main()
{
    CHECK(1, g_and, rt_and_i(-1, 255), -1 & 255);
    CHECK(2, g_and_u, rt_and(-1, 0xF0F0u), -1 & 0xF0F0u);
    CHECK(3, g_or_u, rt_or_u(0x80000000u, 1), 0x80000000u | 1);
    CHECK(4, g_or_neg, rt_or_i(-256, 15), -256 | 15);
    CHECK(5, g_xor_u, rt_xor_u(~0u, 0xF0F0F0F0u), ~0u ^ 0xF0F0F0F0u);
    CHECK(6, g_xor, rt_xor_i(-1, 5), -1 ^ 5);
    CHECK(7, g_xor_mix, rt_xor_u(-1, 0), -1 ^ 0u);
    CHECK(8, g_not_zero, rt_not_i(0), ~0);
    CHECK(9, g_not_u, rt_not_u(0), ~0u);
    CHECK(10, g_not_max, rt_not_i(2147483647), -2147483647 - 1);
    CHECK(11, g_not_wrap, rt_not_u(0), 0xFFFFFFFFu);
    CHECK(12, g_neg_u, rt_neg_u(1), -1u);
    CHECK(13, g_neg_u2, rt_neg_u(0x80000000u), 0x80000000u);
    CHECK(14, g_neg, rt_neg_i(5), -5);
    CHECK(15, g_neg_shl, rt_neg_i(-2147483647 - 1), -2147483647 - 1);
    CHECK(16, g_lnot_five, rt_lnot_i(5), 0);
    CHECK(17, g_lnot_zero, rt_lnot_i(0), 1);
    CHECK(18, g_lnot_umax, rt_lnot_u(0u - 1), 0);
    CHECK(19, g_lnot_wrap, rt_lnot_i(0), 1);
    CHECK(20, g_lnot_uwrap, rt_lnot_u(0), 1);
    CHECK(21, g_plus_u, rt_plus_u(0u - 1), +(0u - 1));
    CHECK(22, g_plus, rt_plus_i(7), 7);
    return 0;
}
