/* Structs among doubles once fa0-fa7 are used up: the 8 doubles fill fa0-fa7, so
   the ninth double goes in two a registers, and the structs after it take a
   registers too: {float,float} in a0,a1 (integer convention), the double d9 in
   a2,a3, {int,int} in a4,a5, a 12-byte struct by reference in a6, and a
   {double} struct in a7 plus the first stack word (the split).
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct FF { float a; float b; };
struct P8 { int a; int b; };
struct Big { int a; int b; int c; };

double ours_mix(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FF s, double d9, struct P8 p, struct Big b, int tail)
{ return d1 + d8 + s.a * 10.0 + s.b + d9 * 100.0 + p.a * 1000.0 + p.b + b.a + b.c + tail; }

double theirs_mix(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FF s, double d9, struct P8 p, struct Big b, int tail);

double call_0(void)
{
    struct FF s; struct P8 p; struct Big b; s.a = 1.5f; s.b = 2.5f; p.a = 3; p.b = 4; b.a = 5; b.b = 6; b.c = 7;
    return theirs_mix(1, 2, 3, 4, 5, 6, 7, 8, s, 0.5, p, b, 100);
}
