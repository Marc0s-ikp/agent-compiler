/* Folded unsigned and mixed signed/unsigned arithmetic. */
unsigned g_add = 4294967295u + 1u;
unsigned g_sub = 0u - 1;
unsigned g_mul = 65536u * 65536u;
unsigned g_mul_max = 4294967295u * 4294967295u;
unsigned g_div = (0u - 1) / 3;
unsigned g_div_half = (0u - 2) / 2;
unsigned g_mod = (0u - 1) % 7;
unsigned g_mix_div = -1 / 2u;
unsigned g_mix_mod = -7 % 4u;
unsigned g_mix_add = -1 + 1u;
unsigned g_mix_mul = -2 * 3u;
unsigned g_mix_sub = 1 - 2u;
int g_mix_cast = (int)(0u - 1) / 2;

unsigned rt_add(unsigned a, unsigned b) { return a + b; }
unsigned rt_sub(unsigned a, unsigned b) { return a - b; }
unsigned rt_mul(unsigned a, unsigned b) { return a * b; }
unsigned rt_div(unsigned a, unsigned b) { return a / b; }
unsigned rt_mod(unsigned a, unsigned b) { return a % b; }
unsigned rt_mix_div(int a, unsigned b) { return a / b; }
unsigned rt_mix_mod(int a, unsigned b) { return a % b; }
unsigned rt_mix_add(int a, unsigned b) { return a + b; }
unsigned rt_mix_mul(int a, unsigned b) { return a * b; }
unsigned rt_mix_sub(int a, unsigned b) { return a - b; }
int rt_int_div(int a, int b) { return a / b; }
