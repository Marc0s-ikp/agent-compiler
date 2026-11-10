extern int g_u_gt, g_mix_lt, g_mix_gt, g_mix_le, g_mix_ge, g_mix_eq, g_mix_ne;
extern int g_s_lt, g_wrap_lt, g_wrap_gt, g_wrap_le, g_wrap_ge, g_wrap_eq;
extern int g_wrap_ne, g_uwrap_eq, g_uwrap_lt;
int rt_mix_lt(int a, unsigned b);
int rt_mix_gt(int a, unsigned b);
int rt_mix_le(int a, unsigned b);
int rt_mix_ge(int a, unsigned b);
int rt_mix_eq(int a, unsigned b);
int rt_mix_ne(int a, unsigned b);
int rt_uu_gt(unsigned a, unsigned b);
int rt_ii_lt(int a, int b);
int rt_ii_gt(int a, int b);
int rt_ii_le(int a, int b);
int rt_ii_ge(int a, int b);
int rt_ii_eq(int a, int b);
int rt_ii_ne(int a, int b);

#define IMIN (-2147483647 - 1)
/* n: folded value wrong; n+100: run-time value wrong. The run-time call gets
   the already wrapped operands. */
#define CHECK(n, folded, runtime, want) \
    do { if ((folded) != (want)) return n; if ((runtime) != (want)) return n + 100; } while (0)

int main()
{
    CHECK(1, g_u_gt, rt_uu_gt(0u - 1, 0), (0u - 1) > 0);
    CHECK(2, g_mix_lt, rt_mix_lt(-1, 0), -1 < 0u);
    CHECK(3, g_mix_gt, rt_mix_gt(-1, 1), -1 > 1u);
    CHECK(4, g_mix_le, rt_mix_le(-1, 0), -1 <= 0u);
    CHECK(5, g_mix_ge, rt_mix_ge(-1, 0), -1 >= 0u);
    CHECK(6, g_mix_eq, rt_mix_eq(-1, 4294967295u), -1 == 4294967295u);
    CHECK(7, g_mix_ne, rt_mix_ne(-1, 0), -1 != 0u);
    CHECK(8, g_s_lt, rt_ii_lt(-1, 0), -1 < 0);
    CHECK(9, g_wrap_lt, rt_ii_lt(IMIN, 0), 1);
    CHECK(10, g_wrap_gt, rt_ii_gt(IMIN, 0), 0);
    CHECK(11, g_wrap_le, rt_ii_le(IMIN, IMIN), 1);
    CHECK(12, g_wrap_ge, rt_ii_ge(IMIN, 0), 0);
    CHECK(13, g_wrap_eq, rt_ii_eq(0, 0), 1);
    CHECK(14, g_wrap_ne, rt_ii_ne(0, 0), 0);
    CHECK(15, g_uwrap_eq, rt_ii_eq(0, 0), 1);
    CHECK(16, g_uwrap_lt, rt_ii_lt(0, 1), 1);
    return 0;
}
