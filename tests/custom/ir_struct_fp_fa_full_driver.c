struct FF { float a; float b; };
struct FI { float f; int i; };
struct DD { double a; double b; };
struct D1 { double a; };

double ours_e8ff(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FF s, int tail);
double ours_e8fi(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FI s, int tail);
double ours_e8dd(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct DD s, int tail);
double ours_e8d1(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, int x, struct D1 s, int tail);
double ours_e7ff(double d1, double d2, double d3, double d4, double d5, double d6, double d7, struct FF s, int tail);
double ours_e7fi(double d1, double d2, double d3, double d4, double d5, double d6, double d7, struct FI s, int tail);
double call_0(void);
double call_1(void);
double call_2(void);
double call_3(void);
double call_4(void);
double call_5(void);

double theirs_e8ff(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FF s, int tail)
{ return d1 + d8 + s.a * 100.0 + s.b * 10.0 + tail; }

double theirs_e8fi(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FI s, int tail)
{ return d1 + d8 + s.f * 100.0 + s.i * 10.0 + tail; }

double theirs_e8dd(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct DD s, int tail)
{ return d1 + d8 + s.a * 100.0 + s.b * 10.0 + tail; }

double theirs_e8d1(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, int x, struct D1 s, int tail)
{ return d1 + d8 + x + s.a * 100.0 + tail; }

double theirs_e7ff(double d1, double d2, double d3, double d4, double d5, double d6, double d7, struct FF s, int tail)
{ return d1 + d7 + s.a * 100.0 + s.b * 10.0 + tail; }

double theirs_e7fi(double d1, double d2, double d3, double d4, double d5, double d6, double d7, struct FI s, int tail)
{ return d1 + d7 + s.f * 100.0 + s.i * 10.0 + tail; }

int main(void)
{
    { struct FF s; s.a = 1.5f; s.b = 2.5f; if ((ours_e8ff(1, 2, 3, 4, 5, 6, 7, 8, s, 9)) != (1 + 8 + 150 + 25 + 9 + 0)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (1 + 8 + 150 + 25 + 9 + 0)) return 2;   /* our caller, GCC callee */
    { struct FI s; s.f = 1.5f; s.i = 2; if ((ours_e8fi(1, 2, 3, 4, 5, 6, 7, 8, s, 9)) != (1 + 8 + 150 + 20 + 9 + 0)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (1 + 8 + 150 + 20 + 9 + 0)) return 4;   /* our caller, GCC callee */
    { struct DD s; s.a = 1.5; s.b = 2.5; if ((ours_e8dd(1, 2, 3, 4, 5, 6, 7, 8, s, 9)) != (1 + 8 + 150 + 25 + 9 + 0)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (1 + 8 + 150 + 25 + 9 + 0)) return 6;   /* our caller, GCC callee */
    { struct D1 s; s.a = 1.5; if ((ours_e8d1(1, 2, 3, 4, 5, 6, 7, 8, 40, s, 9)) != (1 + 8 + 40 + 150 + 9 + 0)) return 7; }   /* GCC caller, our callee */
    if (call_3() != (1 + 8 + 40 + 150 + 9 + 0)) return 8;   /* our caller, GCC callee */
    { struct FF s; s.a = 1.5f; s.b = 2.5f; if ((ours_e7ff(1, 2, 3, 4, 5, 6, 7, s, 9)) != (1 + 7 + 150 + 25 + 9 + 0)) return 9; }   /* GCC caller, our callee */
    if (call_4() != (1 + 7 + 150 + 25 + 9 + 0)) return 10;   /* our caller, GCC callee */
    { struct FI s; s.f = 1.5f; s.i = 2; if ((ours_e7fi(1, 2, 3, 4, 5, 6, 7, s, 9)) != (1 + 7 + 150 + 20 + 9 + 0)) return 11; }   /* GCC caller, our callee */
    if (call_5() != (1 + 7 + 150 + 20 + 9 + 0)) return 12;   /* our caller, GCC callee */
    return 0;
}
