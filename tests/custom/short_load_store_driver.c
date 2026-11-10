extern short gs; extern unsigned short gu; extern short gs_arr[3]; extern unsigned short gu_arr[3];
extern int get_gs(void), get_gu(void), get_gs_arr(int i), get_gu_arr(int i), get_zero(void);
extern int local_neg(void), local_u(void), wrap_s(int v), wrap_u(int v), wrap_global(int v);
extern int store_neighbours(void), compound_s(int v), compound_u(int v), compound_ops(void);
extern int incdec(void), incdec_value(void), incdec_ptr(short *p), incdec_array(void), assign_value(void);

int main(void)
{
    short p;
    if (get_gs() != -2 || get_gu() != 65535) return 1;
    if (get_gs_arr(0) != -1 || get_gs_arr(1) != 32767 || get_gs_arr(2) != -32768) return 2;
    if (get_gu_arr(0) != 1 || get_gu_arr(1) != 65535 || get_gu_arr(2) != 40000) return 3;
    if (get_zero() != 0) return 4;
    if (local_neg() != -300 || local_u() != 65535) return 5;
    if (wrap_s(32768) != -32768 || wrap_s(65535) != -1 || wrap_s(-70000) != -4464) return 6;
    if (wrap_u(65536) != 0 || wrap_u(-1) != 65535 || wrap_u(70000) != 4464) return 7;
    if (wrap_global(-5) != -500000 + 1 || gs != -5 || gu != 65531) return 8;
    if (wrap_global(65536) != 0 || gs != 0 || gu != 0) return 9;
    if (!store_neighbours()) return 10;
    if (compound_s(3000) != -32536 || compound_s(-1) != 29999) return 11;
    if (compound_u(1000) != 464 || compound_u(500) != 65500) return 12;
    {
        int s = 100 * 400;
        s = (short)s;       /* -25536 */
        s -= 1; s /= 3; s %= 1000;
        s = (short)(s << 2); s >>= 1; s |= 1; s &= 0x7ff; s ^= 0x10;
        if (compound_ops() != s) return 13;
    }
    if (incdec() != 15) return 14;
    if (incdec_value() != -1000 + 100 + 10 - 1) return 15;
    p = -7;
    if (incdec_ptr(&p) != -6 || p != -6) return 16;
    p = 32767;
    if (incdec_ptr(&p) != -32768) return 17;
    if (!incdec_array()) return 18;
    if (assign_value() != 44641) return 19;
    return 0;
}
