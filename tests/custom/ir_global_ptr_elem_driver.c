int r0(void);
int r1(void);
int r2(void);
int r3(void);
int r4(void);
int r5(void);
int get(int i);
int chars(void);
void w(void);

int main()
{
    if (r0() != 30 || r1() != 20 || r2() != 40 || r3() != 10) return 1;
    if (r4() != 5 || r5() != 2) return 2;
    if (chars() != 14) return 3;
    w();
    if (get(2) != 31 || get(1) != 21 || get(4) != 51 || get(3) != 41) return 4;
    if (get(0) != 10) return 5;
    if (r0() != 31) return 6;
    return 0;
}
