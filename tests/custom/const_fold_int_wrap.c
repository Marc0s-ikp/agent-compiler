/* Folded int arithmetic must wrap to 32 bits like the run-time code does. */
int g_add = 2147483647 + 1;
int g_sub = -2147483647 - 1 - 1;
int g_mul_big = 65536 * 65536;
int g_mul_mid = 100000 * 100000;
int g_mul_max2 = 2147483647 * 2;
int g_neg_min = -(-2147483647 - 1);
int g_div_min = (-2147483647 - 1) / -1;
int g_mod_min = (-2147483647 - 1) % -1;
int g_shl = 1 << 31;
int g_div_neg = -7 / 2;
int g_mod_neg = -7 % 2;
int g_mod_negd = 7 % -2;

int rt_add(int a, int b) { return a + b; }
int rt_sub(int a, int b) { return a - b; }
int rt_mul(int a, int b) { return a * b; }
int rt_div(int a, int b) { return a / b; }
int rt_mod(int a, int b) { return a % b; }
int rt_shl(int a, int n) { return a << n; }
int rt_neg(int a) { return -a; }
