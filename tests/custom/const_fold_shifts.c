/* Folded shifts: logical vs arithmetic right shift, and the result type. */
unsigned g_shr_all = ~0u >> 1;
unsigned g_shr_28 = ~0u >> 28;
unsigned g_ushr_neg = (0u - 16) >> 2;
int g_sar_neg = -16 >> 2;
int g_sar_all = -1 >> 31;
int g_sar_count_u = -16 >> 2u;
unsigned g_shl_u = 1u << 31;
unsigned g_shl_u_shr = 1u << 31 >> 31;
unsigned g_hi_shr = 0x80000000u >> 31;
int g_shl_sar = (1 << 31) >> 31;
int g_shl_int = 1 << 31;
int g_sar_min = (-2147483647 - 1) >> 4;

unsigned rt_ushr(unsigned a, int n) { return a >> n; }
int rt_sar(int a, int n) { return a >> n; }
int rt_sar_u(int a, unsigned n) { return a >> n; }
unsigned rt_ushl(unsigned a, int n) { return a << n; }
int rt_shl(int a, int n) { return a << n; }
