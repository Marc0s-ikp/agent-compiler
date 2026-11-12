double u2d(unsigned u);
float u2f(unsigned u);
double local_u2d(void);
double cast_u2d(unsigned x);
double compound_u2d(unsigned x);
double ushort_u2d(unsigned short s);
double uchar_u2d(unsigned char c);
double schar_s2d(signed char c);
double int_s2d(int i);
int u_above_signed(unsigned x);

int main(void)
{
    if (u2d(3000000000u) != 3000000000.0) return 1;
    if (u2d(0xFFFFFFFFu) != 4294967295.0) return 2;
    if (u2d(0x80000000u) != 2147483648.0) return 3;
    if (u2d(0x7FFFFFFFu) != 2147483647.0) return 4;
    if (u2d(0u) != 0.0) return 5;
    if (u2f(3000000000u) != 3000000000.0f) return 6;
    if (u2f(0xFFFFFFFFu) != 4294967296.0f) return 7;
    if (u2f(16777217u) != 16777216.0f) return 8;
    if (local_u2d() != 3000000000.0) return 9;
    if (cast_u2d(0xFFFFFFFFu) != 2147483647.5) return 10;
    if (compound_u2d(3000000000u) != 3000000001.0) return 11;
    if (ushort_u2d(65535) != 65535.0) return 12;
    if (uchar_u2d(255) != 255.0) return 13;
    if (schar_s2d(-3) != -3.0) return 14;
    if (int_s2d(-2147483647 - 1) != -2147483648.0) return 15;
    if (u_above_signed(0x80000000u) != 1) return 16;
    if (u_above_signed(5u) != 0) return 17;
    return 0;
}
