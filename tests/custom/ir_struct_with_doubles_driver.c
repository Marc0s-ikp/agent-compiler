struct FF { float a; float b; };
struct P8 { int a; int b; };
struct Big { int a; int b; int c; };

double ours_mix(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FF s, double d9, struct P8 p, struct Big b, int tail);
double call_0(void);

double theirs_mix(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FF s, double d9, struct P8 p, struct Big b, int tail)
{ return d1 + d8 + s.a * 10.0 + s.b + d9 * 100.0 + p.a * 1000.0 + p.b + b.a + b.c + tail; }

int main(void)
{
    { struct FF s; struct P8 p; struct Big b; s.a = 1.5f; s.b = 2.5f; p.a = 3; p.b = 4; b.a = 5; b.b = 6; b.c = 7; if ((ours_mix(1, 2, 3, 4, 5, 6, 7, 8, s, 0.5, p, b, 100)) != (1 + 8 + 15 + 2.5 + 50 + 3000 + 4 + 5 + 7 + 100)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (1 + 8 + 15 + 2.5 + 50 + 3000 + 4 + 5 + 7 + 100)) return 2;   /* our caller, GCC callee */
    return 0;
}
