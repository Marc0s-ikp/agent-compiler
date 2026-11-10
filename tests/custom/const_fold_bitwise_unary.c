/* Folded & | ^ and unary - ~ ! +, signed and unsigned. */
int g_and = -1 & 255;
unsigned g_and_u = -1 & 0xF0F0u;
unsigned g_or_u = 0x80000000u | 1;
int g_or_neg = -256 | 15;
unsigned g_xor_u = ~0u ^ 0xF0F0F0F0u;
int g_xor = -1 ^ 5;
unsigned g_xor_mix = -1 ^ 0u;
int g_not_zero = ~0;
unsigned g_not_u = ~0u;
int g_not_max = ~2147483647;
unsigned g_not_wrap = ~(65536u * 65536u);
unsigned g_neg_u = -1u;
unsigned g_neg_u2 = -(1u << 31);
int g_neg = -5;
int g_neg_shl = -(1 << 31);
int g_lnot_five = !5;
int g_lnot_zero = !0;
int g_lnot_umax = !(0u - 1);
int g_lnot_wrap = !(65536 * 65536);
int g_lnot_uwrap = !(4294967295u + 1);
unsigned g_plus_u = +(0u - 1);
int g_plus = +7;

unsigned rt_and(int a, unsigned b) { return a & b; }
int rt_and_i(int a, int b) { return a & b; }
int rt_or_i(int a, int b) { return a | b; }
unsigned rt_or_u(unsigned a, int b) { return a | b; }
unsigned rt_xor_u(unsigned a, unsigned b) { return a ^ b; }
int rt_xor_i(int a, int b) { return a ^ b; }
int rt_not_i(int a) { return ~a; }
unsigned rt_not_u(unsigned a) { return ~a; }
unsigned rt_neg_u(unsigned a) { return -a; }
int rt_neg_i(int a) { return -a; }
int rt_lnot_i(int a) { return !a; }
int rt_lnot_u(unsigned a) { return !a; }
unsigned rt_plus_u(unsigned a) { return +a; }
int rt_plus_i(int a) { return +a; }
