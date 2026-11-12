/* The floating registers are used up: 8 doubles fill fa0-fa7. A float struct then
   falls back to the integer convention ({float,float} and {float,int} in a0,a1,
   {double,double} by reference). With 7 doubles, {float,float} needs two fa
   registers and only fa7 is free, so it also falls back, but {float,int} still
   fits (fa7 and a0).
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct FF { float a; float b; };
struct FI { float f; int i; };
struct DD { double a; double b; };
struct D1 { double a; };

double ours_e8ff(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FF s, int tail)
{ return d1 + d8 + s.a * 100.0 + s.b * 10.0 + tail; }

double ours_e8fi(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FI s, int tail)
{ return d1 + d8 + s.f * 100.0 + s.i * 10.0 + tail; }

double ours_e8dd(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct DD s, int tail)
{ return d1 + d8 + s.a * 100.0 + s.b * 10.0 + tail; }

double ours_e8d1(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, int x, struct D1 s, int tail)
{ return d1 + d8 + x + s.a * 100.0 + tail; }

double ours_e7ff(double d1, double d2, double d3, double d4, double d5, double d6, double d7, struct FF s, int tail)
{ return d1 + d7 + s.a * 100.0 + s.b * 10.0 + tail; }

double ours_e7fi(double d1, double d2, double d3, double d4, double d5, double d6, double d7, struct FI s, int tail)
{ return d1 + d7 + s.f * 100.0 + s.i * 10.0 + tail; }

double theirs_e8ff(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FF s, int tail);
double theirs_e8fi(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct FI s, int tail);
double theirs_e8dd(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, struct DD s, int tail);
double theirs_e8d1(double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, int x, struct D1 s, int tail);
double theirs_e7ff(double d1, double d2, double d3, double d4, double d5, double d6, double d7, struct FF s, int tail);
double theirs_e7fi(double d1, double d2, double d3, double d4, double d5, double d6, double d7, struct FI s, int tail);

double call_0(void)
{
    struct FF s; s.a = 1.5f; s.b = 2.5f;
    return theirs_e8ff(1, 2, 3, 4, 5, 6, 7, 8, s, 9);
}

double call_1(void)
{
    struct FI s; s.f = 1.5f; s.i = 2;
    return theirs_e8fi(1, 2, 3, 4, 5, 6, 7, 8, s, 9);
}

double call_2(void)
{
    struct DD s; s.a = 1.5; s.b = 2.5;
    return theirs_e8dd(1, 2, 3, 4, 5, 6, 7, 8, s, 9);
}

double call_3(void)
{
    struct D1 s; s.a = 1.5;
    return theirs_e8d1(1, 2, 3, 4, 5, 6, 7, 8, 40, s, 9);
}

double call_4(void)
{
    struct FF s; s.a = 1.5f; s.b = 2.5f;
    return theirs_e7ff(1, 2, 3, 4, 5, 6, 7, s, 9);
}

double call_5(void)
{
    struct FI s; s.f = 1.5f; s.i = 2;
    return theirs_e7fi(1, 2, 3, 4, 5, 6, 7, s, 9);
}
