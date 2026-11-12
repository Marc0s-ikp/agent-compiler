unsigned d2u(double d);
unsigned f2u(float f);
int d2i(double d);
int f2i(float f);
unsigned cast_d2u(double d);
unsigned local_d2u(void);
unsigned char d2uc(double d);
signed char d2sc(double d);
unsigned short d2us(double d);
short d2ss(double d);
int d2u_gt(double d);

int main(void)
{
    if (d2u(3000000000.0) != 3000000000u) return 1;
    if (d2u(4294967295.0) != 0xFFFFFFFFu) return 2;
    if (d2u(2147483648.0) != 0x80000000u) return 3;
    if (d2u(2147483647.9) != 2147483647u) return 4;
    if (d2u(0.5) != 0u) return 5;
    if (f2u(3000000000.0f) != 3000000000u) return 6;
    if (f2u(2147483648.0f) != 0x80000000u) return 7;
    if (cast_d2u(3999999999.5) != 3999999999u) return 8;
    if (local_d2u() != 3000000000u) return 9;
    if (d2i(-2.7) != -2) return 10;
    if (d2i(2.7) != 2) return 11;
    if (f2i(-2147483648.0f) != -2147483647 - 1) return 12;
    if (d2uc(200.7) != 200) return 13;
    if (d2sc(-3.9) != -3) return 14;
    if (d2us(65535.9) != 65535) return 15;
    if (d2ss(-32768.5) != -32768) return 16;
    if (d2u_gt(3000000000.0) != 1) return 17;
    if (d2u_gt(5.0) != 0) return 18;
    return 0;
}
