/* short across the compiler boundary: ours_* are compiled here, theirs_* by GCC. */
short ours_neg(short a) { return -a; }
short ours_add(short a, short b) { return a + b; }
unsigned short ours_uadd(unsigned short a, unsigned short b) { return a + b; }
int ours_widen(short a, unsigned short b) { return a * 100000 + b; }
short ours_narrow(int v) { return v; }
unsigned short ours_unarrow(int v) { return v; }
short ours_ten(short a, short b, short c, short d, short e, short f, short g, short h, short i, short j)
{ return a - b + c - d + e - f + g - h + i - j; }
int ours_ten_wide(short a, short b, short c, short d, short e, short f, short g, short h, short i, short j)
{ return (i + j) * 1000 + (g + h) * 10 + a; }
int ours_ten_u(unsigned short a, short b, unsigned short c, short d, unsigned short e, short f,
               unsigned short g, short h, unsigned short i, short j)
{ return (i == 65535) + 2 * (j == -1) + 4 * (g == 40000) + 8 * (h == -32768) + 16 * (a == 1); }
int ours_mixed(signed char c, short s, int i, unsigned short u, signed char d, short t, int k, short m, short n, signed char o)
{ return c + s + i + u + d + t + k + m * 10 + n * 100 + o; }
short ours_chain(short a) { return ours_add(ours_neg(a), ours_add(a, -3)); }

extern short theirs_neg(short a);
extern short theirs_add(short a, short b);
extern unsigned short theirs_uadd(unsigned short a, unsigned short b);
extern int theirs_widen(short a, unsigned short b);
extern short theirs_narrow(int v);
extern unsigned short theirs_unarrow(int v);
extern short theirs_ten(short a, short b, short c, short d, short e, short f, short g, short h, short i, short j);
extern int theirs_ten_wide(short a, short b, short c, short d, short e, short f, short g, short h, short i, short j);
extern int theirs_ten_u(unsigned short a, short b, unsigned short c, short d, unsigned short e, short f,
                        unsigned short g, short h, unsigned short i, short j);
extern int theirs_mixed(signed char c, short s, int i, unsigned short u, signed char d, short t, int k, short m, short n, signed char o);

int call_neg(void) { return theirs_neg(-5) * 100 + theirs_neg(32767); }
int call_add(void) { return theirs_add(-30000, -10000); }              /* wraps to 25536 */
int call_uadd(void) { return theirs_uadd(65535, 2); }
int call_widen(void) { return theirs_widen(-2, 65535); }
int call_narrow(void) { return theirs_narrow(-70000) ; }
int call_unarrow(void) { return theirs_unarrow(-1); }
int call_int_arg(void) { int big = 100000; return theirs_neg(big); }    /* int converted to short parameter */
int call_ten(void) { return theirs_ten(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10); }
int call_ten_wide(void) { return theirs_ten_wide(-1, 2, -3, 4, -5, 6, -7, 8, -9, 10); }
int call_ten_u(void) { return theirs_ten_u(1, 2, 3, 4, 5, 6, 40000, -32768, 65535, -1); }
int call_mixed(void) { return theirs_mixed(-1, -2, -3, 65535, -4, -5, -6, -7, -8, -9); }
int call_nested(void) { return theirs_add(theirs_neg(-4), theirs_add(-1, theirs_neg(2))); }
int call_expr_arg(void) { short s = -9; return theirs_add(s * 2, s + 40000); }
