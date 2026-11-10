struct P { int a; int b; };
struct Big { int a; int b; int c; };
struct F { float x; };
struct D { double x; };

double ours_mixed(int i0, struct P p, float f0, struct Big big, double d0, struct F f,
                  struct D d, int i1, struct P q, int i2, struct Big big2, int i3);
double call_mixed(void);

double theirs_mixed(int i0, struct P p, float f0, struct Big big, double d0, struct F f,
                    struct D d, int i1, struct P q, int i2, struct Big big2, int i3)
{
    return i0 + p.a * 2 + p.b * 3 + f0 + big.a + big.b + big.c + d0 + f.x + d.x
         + i1 + q.a + q.b + i2 + big2.a + big2.c + i3;
}

int main()
{
    struct P p, q; struct Big big, big2; struct F f; struct D d;
    p.a = 1; p.b = 2; q.a = 3; q.b = 4;
    big.a = 5; big.b = 6; big.c = 7; big2.a = 8; big2.b = 9; big2.c = 10;
    f.x = 0.5f; d.x = 0.25;
    /* 100+2+6+1.5+18+2.5+0.5+0.25+11+3+4+12+8+10+13 */
    if (ours_mixed(100, p, 1.5f, big, 2.5, f, d, 11, q, 12, big2, 13) != 191.75) return 1;
    if (call_mixed() != 191.75) return 2;
    return 0;
}
