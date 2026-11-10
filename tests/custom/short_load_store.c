/* Loads and stores of short and unsigned short: sign, wrap, globals,
   compound assignment, ++ and --. */
short gs = -2;
unsigned short gu = 65535;
short gs_arr[3] = {-1, 32767, -32768};
unsigned short gu_arr[3] = {1, 65535, 40000};
short gzero;

int get_gs(void) { return gs; }
int get_gu(void) { return gu; }
int get_gs_arr(int i) { return gs_arr[i]; }
int get_gu_arr(int i) { return gu_arr[i]; }
int get_zero(void) { return gzero; }

int local_neg(void) { short s; s = -300; return s; }
int local_u(void) { unsigned short u; u = 65535; return u; }
int wrap_s(int v) { short s; s = v; return s; }
int wrap_u(int v) { unsigned short u; u = v; return u; }
int wrap_global(int v) { gs = v; gu = v; return gs * 100000 + (gu != 0); }
int store_neighbours(void)
{
    short a[3];
    a[0] = 0x1111; a[1] = -1; a[2] = 0x2222;
    a[1] = 0;
    return a[0] == 0x1111 && a[1] == 0 && a[2] == 0x2222;
}

int compound_s(int v)
{
    short s = 30000;
    s += v;
    return s;
}
int compound_u(int v)
{
    unsigned short u = 65000;
    u += v;
    return u;
}
int compound_ops(void)
{
    short s = 100;
    s *= 400;       /* 40000 wraps to -25536 */
    s -= 1;
    s /= 3;
    s %= 1000;
    s <<= 2;
    s >>= 1;
    s |= 1;
    s &= 0x7ff;
    s ^= 0x10;
    return s;
}
int incdec(void)
{
    short s = 32767;
    unsigned short u = 0;
    int r;
    s++;                 /* -32768 */
    u--;                 /* 65535 */
    r = (s == -32768) + 2 * (u == 65535);
    ++s; --u;            /* -32767, 65534 */
    r += 4 * (s == -32767) + 8 * (u == 65534);
    return r;
}
int incdec_value(void)
{
    short s = -1;
    int a, b, c, d;
    a = s++;  /* a = -1, s = 0 */
    b = ++s;  /* b = 1 */
    c = s--;  /* c = 1, s = 0 */
    d = --s;  /* d = -1 */
    return a * 1000 + b * 100 + c * 10 + d;
}
int incdec_ptr(short *p) { (*p)++; ++*p; (*p)--; return *p; }
int incdec_array(void)
{
    short a[2];
    a[0] = 32767; a[1] = -32768;
    a[0]++; a[1]--;
    return a[0] == -32768 && a[1] == 32767;
}
int assign_value(void)
{
    short s;
    int x = (s = 70000);   /* value of assignment has the type of s */
    return x * 10 + (s == 4464);
}
