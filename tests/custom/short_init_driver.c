extern short g_arr[4], g_part[5], g_auto[3], g_word, g_from_enum, g_neg_const, g_wrap, g_expr, g_2d[2][2];
extern unsigned short g_u[3];
extern int sum_arr(void), part(void), autosize(void), ushorts(void), gp(void), gps(void), misc(void), two_d(void);
extern int local_arr(void), local_part(void), local_auto(void), local_struct(void), local_enum(void);
extern int local_word(void), enum_in_short(void), counter(void), static_arr(void);

int main(void)
{
    if (g_arr[0] != -1 || g_arr[3] != 4 || sum_arr() != 2) return 1;
    if (g_part[1] != -8 || g_part[4] != 0 || part() != 7000 - 8) return 2;
    if (g_auto[0] != -100 || autosize() != 3000 - 300) return 3;
    if (g_u[0] != 65535 || ushorts() != 65535 + 40000 + 1) return 4;
    if (gp() != -500000 + 'x' * 1000 + 100 - 60 + 7) return 5;
    if (gps() != -1 - 3 - 4 - 5 + 2) return 6;
    if (misc() != -77 - 300 + 30000 - 32768 + 4464 - 20) return 7;
    if (g_word != -77 || g_neg_const != -32768 || g_wrap != 4464 || g_expr != -20) return 8;
    if (two_d() != -20 - 3 || g_2d[1][1] != -4) return 9;
    if (local_arr() != -4) return 10;
    if (local_part() != -90) return 11;
    if (local_auto() != 600 - 2) return 12;
    if (local_struct() != -5 + 'y' + 1 - 6 - 7) return 13;
    if (local_enum() != 29700) return 14;
    if (local_word() != -100000 + 65535) return 15;
    if (!enum_in_short()) return 16;
    if (counter() != -1000 + 0) return 17;
    if (counter() != 0 + 1) return 18;
    if (static_arr() != -200 - 20 - 3) return 19;
    if (static_arr() != -300 - 20 - 3) return 20;
    return 0;
}
