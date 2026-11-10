/* Folded comparisons: unsigned vs signed, and operands that wrap first. */
int g_u_gt = (0u - 1) > 0;
int g_mix_lt = -1 < 0u;
int g_mix_gt = -1 > 1u;
int g_mix_le = -1 <= 0u;
int g_mix_ge = -1 >= 0u;
int g_mix_eq = -1 == 4294967295u;
int g_mix_ne = -1 != 0u;
int g_s_lt = -1 < 0;
int g_wrap_lt = 2147483647 + 1 < 0;
int g_wrap_gt = 2147483647 + 1 > 0;
int g_wrap_le = 2147483647 + 1 <= -2147483647 - 1;
int g_wrap_ge = 2147483647 + 1 >= 0;
int g_wrap_eq = 65536 * 65536 == 0;
int g_wrap_ne = 65536 * 65536 != 0;
int g_uwrap_eq = 4294967295u + 1 == 0;
int g_uwrap_lt = 4294967295u + 1 < 1;

int rt_mix_lt(int a, unsigned b) { return a < b; }
int rt_mix_gt(int a, unsigned b) { return a > b; }
int rt_mix_le(int a, unsigned b) { return a <= b; }
int rt_mix_ge(int a, unsigned b) { return a >= b; }
int rt_mix_eq(int a, unsigned b) { return a == b; }
int rt_mix_ne(int a, unsigned b) { return a != b; }
int rt_uu_gt(unsigned a, unsigned b) { return a > b; }
int rt_ii_lt(int a, int b) { return a < b; }
int rt_ii_gt(int a, int b) { return a > b; }
int rt_ii_le(int a, int b) { return a <= b; }
int rt_ii_ge(int a, int b) { return a >= b; }
int rt_ii_eq(int a, int b) { return a == b; }
int rt_ii_ne(int a, int b) { return a != b; }
