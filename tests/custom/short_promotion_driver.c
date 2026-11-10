extern int add_u(void), mul_u(void), mul_s(void), neg_u(void), not_u(void), not_s(void);
extern int shift_s(void), shift_u(void), div_s(void), div_u(void);
extern int cmp_neg_u(void), cmp_cast(void), cmp_s_u(void), cmp_s_ui(void);
extern int cmp_u_eq(void), cmp_s_eq(void), cmp_s_u_eq(void), plus_const(void);
extern int tern(int c), logic(void), cond_loop(void), unsigned_loop(void);
extern int arg_promote_sum(short a, short b), index_by_short(void);
extern double mix_double(void);
extern float mix_float(void);

int main(void)
{
    if (add_u() != 65536) return 1;
    if (mul_u() != 1) return 2;
    if (mul_s() != 1073676289) return 3;
    if (neg_u() != -1 || not_u() != -1 || not_s() != -6) return 4;
    if (shift_s() != -32768 + -1) return 5;
    if (shift_u() != 0x10000 + 1) return 6;
    if (div_s() != -3 * 10 - 1) return 7;
    if (div_u() != 255 * 1000 + 255) return 8;
    if (cmp_neg_u() != 1 || cmp_cast() != 1) return 9;
    if (cmp_s_u() != 1) return 10;
    if (cmp_s_ui() != 0) return 11;
    if (cmp_u_eq() != 1 || cmp_s_eq() != 1) return 12;
    if (cmp_s_u_eq() != 0) return 13;
    if (plus_const() != 65536) return 14;
    if (tern(1) != 97 || tern(0) != 104) return 15;
    if (logic() != 1 + 0 + 4) return 16;
    if (cond_loop() != -3) return 17;
    if (unsigned_loop() != 5) return 18;
    if (arg_promote_sum(32767, 32767) != 65534 || arg_promote_sum(-32768, -1) != -32769) return 19;
    if (index_by_short() != 80 + 6) return 20;
    if (mix_double() != -1.5) return 21;
    if (mix_float() != 65535.5f) return 22;
    return 0;
}
