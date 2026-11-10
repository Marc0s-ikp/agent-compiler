extern short ours_neg(short a);
extern short ours_add(short a, short b);
extern unsigned short ours_uadd(unsigned short a, unsigned short b);
extern int ours_widen(short a, unsigned short b);
extern short ours_narrow(int v);
extern unsigned short ours_unarrow(int v);
extern short ours_ten(short a, short b, short c, short d, short e, short f, short g, short h, short i, short j);
extern int ours_ten_wide(short a, short b, short c, short d, short e, short f, short g, short h, short i, short j);
extern int ours_ten_u(unsigned short a, short b, unsigned short c, short d, unsigned short e, short f,
                      unsigned short g, short h, unsigned short i, short j);
extern int ours_mixed(signed char c, short s, int i, unsigned short u, signed char d, short t, int k, short m, short n, signed char o);
extern short ours_chain(short a);
extern int call_neg(void), call_add(void), call_uadd(void), call_widen(void), call_narrow(void);
extern int call_unarrow(void), call_int_arg(void), call_ten(void), call_ten_wide(void);
extern int call_ten_u(void), call_mixed(void), call_nested(void), call_expr_arg(void);

short theirs_neg(short a) { return -a; }
short theirs_add(short a, short b) { return a + b; }
unsigned short theirs_uadd(unsigned short a, unsigned short b) { return a + b; }
int theirs_widen(short a, unsigned short b) { return a * 100000 + b; }
short theirs_narrow(int v) { return v; }
unsigned short theirs_unarrow(int v) { return v; }
short theirs_ten(short a, short b, short c, short d, short e, short f, short g, short h, short i, short j)
{ return a - b + c - d + e - f + g - h + i - j; }
int theirs_ten_wide(short a, short b, short c, short d, short e, short f, short g, short h, short i, short j)
{ return (i + j) * 1000 + (g + h) * 10 + a; }
int theirs_ten_u(unsigned short a, short b, unsigned short c, short d, unsigned short e, short f,
                 unsigned short g, short h, unsigned short i, short j)
{ return (i == 65535) + 2 * (j == -1) + 4 * (g == 40000) + 8 * (h == -32768) + 16 * (a == 1); }
int theirs_mixed(signed char c, short s, int i, unsigned short u, signed char d, short t, int k, short m, short n, signed char o)
{ return c + s + i + u + d + t + k + m * 10 + n * 100 + o; }

int main(void)
{
    int big = 100000;
    if (ours_neg(-5) != 5 || ours_neg(32767) != -32767 || ours_neg(-32768) != -32768) return 1;
    if (ours_add(-30000, -10000) != 25536 || ours_add(1, -3) != -2) return 2;
    if (ours_uadd(65535, 2) != 1 || ours_uadd(1, 2) != 3) return 3;
    if (ours_widen(-2, 65535) != -200000 + 65535) return 4;
    if (ours_narrow(-70000) != -4464 || ours_narrow(40000) != -25536) return 5;
    if (ours_unarrow(-1) != 65535 || ours_unarrow(70000) != 4464) return 6;
    if (ours_neg(big) != theirs_neg(big) || ours_neg(big) != (short)-(short)big) return 7;
    if (ours_ten(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10) != theirs_ten(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10)) return 8;
    if (ours_ten(1, 2, 3, 4, 5, 6, 7, 8, 9, 10) != -5) return 9;
    if (ours_ten_wide(-1, 2, -3, 4, -5, 6, -7, 8, -9, 10) != theirs_ten_wide(-1, 2, -3, 4, -5, 6, -7, 8, -9, 10)) return 10;
    if (ours_ten_wide(-1, 2, -3, 4, -5, 6, -7, 8, -9, 10) != 1000 + 10 - 1) return 11;
    if (ours_ten_u(1, 2, 3, 4, 5, 6, 40000, -32768, 65535, -1) != 31) return 12;
    if (ours_mixed(-1, -2, -3, 65535, -4, -5, -6, -7, -8, -9)
        != -1 - 2 - 3 + 65535 - 4 - 5 - 6 - 70 - 800 - 9) return 13;
    if (ours_chain(1234) != -3 || ours_chain(-32768) != -3) return 14;

    if (call_neg() != 500 - 32767) return 21;
    if (call_add() != 25536) return 22;
    if (call_uadd() != 1) return 23;
    if (call_widen() != -200000 + 65535) return 24;
    if (call_narrow() != -4464) return 25;
    if (call_unarrow() != 65535) return 26;
    if (call_int_arg() != theirs_neg(big)) return 27;
    if (call_ten() != 5) return 28;
    if (call_ten_wide() != 1000 + 10 - 1) return 29;
    if (call_ten_u() != 31) return 30;
    if (call_mixed() != -1 - 2 - 3 + 65535 - 4 - 5 - 6 - 70 - 800 - 9) return 31;
    if (call_nested() != 4 + (-1 - 2)) return 32;
    if (call_expr_arg() != -18 + (short)(-9 + 40000)) return 33;
    return 0;
}
