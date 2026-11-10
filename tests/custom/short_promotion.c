/* short and unsigned short promote to int in arithmetic and comparison. */
int add_u(void) { unsigned short u = 65535; return u + 1; }
int mul_u(void) { unsigned short u = 65535; return u * 32768 == 2147450880; }
int mul_s(void) { short s = 32767; return s * s; }
int neg_u(void) { unsigned short u = 1; return -u; }
int not_u(void) { unsigned short u = 0; return ~u; }
int not_s(void) { short s = 5; return ~s; }
int shift_s(void) { short s = -1; return (s << 15) + (s >> 3); }
int shift_u(void) { unsigned short u = 0x8000; return (u << 1) + (u >> 15); }
int div_s(void) { short a = -7, b = 2; return (a / b) * 10 + a % b; }
int div_u(void) { unsigned short a = 65535, b = 256; return (a / b) * 1000 + a % b; }
int cmp_neg_u(void) { unsigned short u = 1; return -1 < u; }            /* int compare: true */
int cmp_cast(void) { return -1 < (unsigned short)1; }
int cmp_s_u(void) { short s = -1; unsigned short u = 65535; return s < u; }
int cmp_s_ui(void) { short s = -1; unsigned int u = 1; return s < u; }   /* unsigned int compare: false */
int cmp_u_eq(void) { unsigned short u = 65535; int i = 65535; return u == i; }
int cmp_s_eq(void) { short s = -1; int i = -1; return s == i; }
int cmp_s_u_eq(void) { short s = -1; unsigned short u = 65535; return s == u; }
int plus_const(void) { return (unsigned short)65535 + 1; }
int tern(int c) { short s = -3; unsigned short u = 4; return (c ? s : u) + 100; }
int logic(void) { short s = -1; unsigned short u = 0; return (s && 1) + 2 * (u || 0) + 4 * !u; }
int cond_loop(void)
{
    short i; int n = 0;
    for (i = -3; i < 3; i++) n += i;
    return n;
}
int unsigned_loop(void)
{
    unsigned short i; int n = 0;
    for (i = 65533; i != 2; i++) n++;   /* wraps through 0 */
    return n;
}
int arg_promote_sum(short a, short b) { return a + b; }
int index_by_short(void)
{
    int a[4]; short i; a[0] = 5; a[1] = 6; a[2] = 7; a[3] = 8;
    i = 3;
    return a[i] * 10 + a[i - 2];
}
double mix_double(void) { short s = -3; return s * 0.5; }
float mix_float(void) { unsigned short u = 65535; return u + 0.5f; }
