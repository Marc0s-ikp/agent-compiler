float i2f(int x); double i2d(int x); float u2f(unsigned x); double u2d(unsigned x);
int f2i(float x); int d2i(double x); unsigned f2u(float x); unsigned d2u(double x);
double f2d(float x); float d2f(double x);
int cast_d2i(double x); double cast_i2d(int x); float cast_i2f(int x);
double cast_f2d(float x); float cast_d2f(double x); unsigned cast_d2u(double x);
double cast_u2d(unsigned x);
int c2d(char c); double sc2d(signed char c); int d2c(double x); int d2sc(double x);
int f2c(float x);
int assign_conv(void); float big_add(void); double big_add_d(void);
int neg_trunc(void); int in_expr(int a, double b); int cond_conv(int c, int a, double b);

int main()
{
    if (i2f(-5) != -5.0f) return 1;
    if (i2d(-5) != -5.0) return 2;
    if (i2d(2147483647) != 2147483647.0) return 3;
    if (i2f(16777217) != 16777216.0f) return 4;
    if (u2f(3000000000u) != 3000000000.0f) return 5;
    if (u2d(4000000000u) != 4000000000.0) return 6;
    if (u2d(7u) != 7.0) return 7;
    if (f2i(2.9f) != 2) return 8;
    if (f2i(-2.9f) != -2) return 9;
    if (d2i(-1234.75) != -1234) return 10;
    if (d2i(2147483520.0) != 2147483520) return 11;
    if (f2u(3000000000.0f) != 3000000000u) return 12;
    if (d2u(4000000000.0) != 4000000000u) return 13;
    if (d2u(5.9) != 5u) return 14;
    if (f2d(0.1f) == 0.1) return 15;          /* widening keeps the float error */
    if (f2d(1.5f) != 1.5) return 16;
    if (d2f(0.1) != 0.1f) return 17;
    if (d2f(1e10) != 1e10f) return 18;
    if (cast_d2i(9.99) != 9) return 19;
    if (cast_i2d(-3) != -3.0) return 20;
    if (cast_i2f(3) != 3.0f) return 21;
    if (cast_f2d(2.5f) != 2.5) return 22;
    if (cast_d2f(2.5) != 2.5f) return 23;
    if (cast_d2u(250.5) != 250u) return 24;
    if (cast_u2d(4294967295u) != 4294967295.0) return 25;
    if (c2d(100) != 200) return 26;
    if (c2d(200) != 400) return 27;            /* plain char is unsigned (D8) */
    if (sc2d(-3) != -3.0) return 28;
    if (d2c(65.9) != 65) return 29;
    if (d2sc(-3.9) != -3) return 30;
    if (f2c(66.5f) != 66) return 31;
    if (assign_conv() != 475) return 32;
    if (big_add() != 16777216.0f) return 33;
    if (big_add_d() != 16777217.0) return 34;
    if (neg_trunc() != -2) return 35;
    if (in_expr(3, 1.5) != 5) return 36;
    if (cond_conv(1, 7, 2.5) != 7) return 37;
    if (cond_conv(0, 7, 2.5) != 2) return 38;
    return 0;
}
