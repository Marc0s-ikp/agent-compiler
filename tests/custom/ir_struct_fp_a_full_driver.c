struct FI { float f; int i; };
struct ID { int i; double d; };
struct FF { float a; float b; };

double ours_a7fi(int a, int b, int c, int d, int e, int f, int g, struct FI s, int tail);
double ours_a8fi(int a, int b, int c, int d, int e, int f, int g, int h, struct FI s, int tail);
double ours_a7id(int a, int b, int c, int d, int e, int f, int g, struct ID s, int tail);
double ours_a8id(int a, int b, int c, int d, int e, int f, int g, int h, struct ID s, int tail);
double ours_a8ff(int a, int b, int c, int d, int e, int f, int g, int h, struct FF s, int tail);
double call_0(void);
double call_1(void);
double call_2(void);
double call_3(void);
double call_4(void);

double theirs_a7fi(int a, int b, int c, int d, int e, int f, int g, struct FI s, int tail)
{ return a + g + s.f * 100.0 + s.i * 10.0 + tail; }

double theirs_a8fi(int a, int b, int c, int d, int e, int f, int g, int h, struct FI s, int tail)
{ return a + h + s.f * 100.0 + s.i * 10.0 + tail; }

double theirs_a7id(int a, int b, int c, int d, int e, int f, int g, struct ID s, int tail)
{ return a + g + s.d * 100.0 + s.i * 10.0 + tail; }

double theirs_a8id(int a, int b, int c, int d, int e, int f, int g, int h, struct ID s, int tail)
{ return a + h + s.d * 100.0 + s.i * 10.0 + tail; }

double theirs_a8ff(int a, int b, int c, int d, int e, int f, int g, int h, struct FF s, int tail)
{ return a + h + s.a * 100.0 + s.b * 10.0 + tail; }

int main(void)
{
    { struct FI s; s.f = 1.5f; s.i = 2; if ((ours_a7fi(1, 2, 3, 4, 5, 6, 7, s, 9)) != (1 + 7 + 150 + 20 + 9 + 0)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (1 + 7 + 150 + 20 + 9 + 0)) return 2;   /* our caller, GCC callee */
    { struct FI s; s.f = 1.5f; s.i = 2; if ((ours_a8fi(1, 2, 3, 4, 5, 6, 7, 8, s, 9)) != (1 + 8 + 150 + 20 + 9 + 0)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (1 + 8 + 150 + 20 + 9 + 0)) return 4;   /* our caller, GCC callee */
    { struct ID s; s.i = 2; s.d = 1.5; if ((ours_a7id(1, 2, 3, 4, 5, 6, 7, s, 9)) != (1 + 7 + 150 + 20 + 9 + 0)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (1 + 7 + 150 + 20 + 9 + 0)) return 6;   /* our caller, GCC callee */
    { struct ID s; s.i = 2; s.d = 1.5; if ((ours_a8id(1, 2, 3, 4, 5, 6, 7, 8, s, 9)) != (1 + 8 + 150 + 20 + 9 + 0)) return 7; }   /* GCC caller, our callee */
    if (call_3() != (1 + 8 + 150 + 20 + 9 + 0)) return 8;   /* our caller, GCC callee */
    { struct FF s; s.a = 1.5f; s.b = 2.5f; if ((ours_a8ff(1, 2, 3, 4, 5, 6, 7, 8, s, 9)) != (1 + 8 + 150 + 25 + 9 + 0)) return 9; }   /* GCC caller, our callee */
    if (call_4() != (1 + 8 + 150 + 25 + 9 + 0)) return 10;   /* our caller, GCC callee */
    return 0;
}
