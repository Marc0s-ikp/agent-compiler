extern short r1(short a);
extern short int r2(short int a);
extern signed short r3(signed short a);
extern signed short int r4(signed short int a);
extern unsigned short r5(unsigned short a);
extern unsigned short int r6(unsigned short int a);
extern int locals(int v), sizes(void);

int main(void)
{
    if (r1(-5) != -5 || r2(-6) != -6 || r3(-7) != -7 || r4(-8) != -8) return 1;
    if (r1(32767) != 32767 || r4(-32768) != -32768) return 2;
    if (r5(65535) != 65535 || r6(65535) != 65535) return 3;
    if (r5(70000) != 4464 || r6(-1) != 65535) return 4;
    if (r1(40000) != -25536) return 5;
    if (locals(7) != 14) return 6;
    if (locals(-3) != -6 + 1) return 7;
    if (locals(40000) != -25536 * 2 + 1) return 8;
    if (sizes() != 222222) return 9;
    return 0;
}
