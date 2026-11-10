extern unsigned g_add, g_sub, g_mul, g_mul_max, g_div, g_div_half, g_mod;
extern unsigned g_mix_div, g_mix_mod, g_mix_add, g_mix_mul, g_mix_sub;
extern int g_mix_cast;
unsigned rt_add(unsigned a, unsigned b);
unsigned rt_sub(unsigned a, unsigned b);
unsigned rt_mul(unsigned a, unsigned b);
unsigned rt_div(unsigned a, unsigned b);
unsigned rt_mod(unsigned a, unsigned b);
unsigned rt_mix_div(int a, unsigned b);
unsigned rt_mix_mod(int a, unsigned b);
unsigned rt_mix_add(int a, unsigned b);
unsigned rt_mix_mul(int a, unsigned b);
unsigned rt_mix_sub(int a, unsigned b);
int rt_int_div(int a, int b);

#define UMAX 4294967295u
/* n: folded value wrong; n+100: run-time value wrong. */
#define CHECK(n, folded, runtime, want) \
    do { if ((folded) != (want)) return n; if ((runtime) != (want)) return n + 100; } while (0)

int main()
{
    CHECK(1, g_add, rt_add(UMAX, 1), UMAX + 1u);
    CHECK(2, g_sub, rt_sub(0, 1), 0u - 1);
    CHECK(3, g_mul, rt_mul(65536, 65536), 65536u * 65536u);
    CHECK(4, g_mul_max, rt_mul(UMAX, UMAX), UMAX * UMAX);
    CHECK(5, g_div, rt_div(UMAX, 3), (0u - 1) / 3);
    CHECK(6, g_div_half, rt_div(UMAX - 1, 2), (0u - 2) / 2);
    CHECK(7, g_mod, rt_mod(UMAX, 7), (0u - 1) % 7);
    CHECK(8, g_mix_div, rt_mix_div(-1, 2), -1 / 2u);
    CHECK(9, g_mix_mod, rt_mix_mod(-7, 4), -7 % 4u);
    CHECK(10, g_mix_add, rt_mix_add(-1, 1), -1 + 1u);
    CHECK(11, g_mix_mul, rt_mix_mul(-2, 3), -2 * 3u);
    CHECK(12, g_mix_sub, rt_mix_sub(1, 2), 1 - 2u);
    CHECK(13, g_mix_cast, rt_int_div(-1, 2), (int)(0u - 1) / 2);
    return 0;
}
