/* When the fa registers run out, a float struct falls back to the integer
   convention: <= 8 bytes in a registers, > 8 bytes by reference.
   d1..d8 are doubles that fill fa0-fa7 (or fa0-fa6 in the 7-double case). */
struct FF { float a; float b; };
struct FI { float f; int i; };
struct DD { double a; double b; };
struct D1 { double a; };

/* 7 doubles: FF needs two fa, only fa7 is free -> a0,a1; tail in a2 */
double ours_ff(double d1, double d2, double d3, double d4, double d5, double d6,
               double d7, struct FF s, int tail)
{ return d1 + d7 + s.a * 100.0 + s.b * 10.0 + tail; }

/* 7 doubles: FI needs fa7 and a0 -> fits the FP convention, tail in a1 */
double ours_fi7(double d1, double d2, double d3, double d4, double d5, double d6,
                double d7, struct FI s, int tail)
{ return d1 + d7 + s.f * 100.0 + s.i * 10.0 + tail; }

/* 8 doubles: FI goes to a0,a1; tail in a2 */
double ours_fi8(double d1, double d2, double d3, double d4, double d5, double d6,
                double d7, double d8, struct FI s, int tail)
{ return d1 + d8 + s.f * 100.0 + s.i * 10.0 + tail; }

/* 8 doubles: DD (16 bytes) by reference in a0; tail in a1 */
double ours_dd(double d1, double d2, double d3, double d4, double d5, double d6,
               double d7, double d8, struct DD s, int tail)
{ return d1 + d8 + s.a * 100.0 + s.b * 10.0 + tail; }

/* 8 doubles: x in a0, struct { double } in a1,a2 (not aligned to an even
   register), tail in a3 */
double ours_d1(double d1, double d2, double d3, double d4, double d5, double d6,
               double d7, double d8, int x, struct D1 s, int tail)
{ return d1 + d8 + x + s.a * 100.0 + tail; }

double theirs_ff(double d1, double d2, double d3, double d4, double d5, double d6,
                 double d7, struct FF s, int tail);
double theirs_fi7(double d1, double d2, double d3, double d4, double d5, double d6,
                  double d7, struct FI s, int tail);
double theirs_fi8(double d1, double d2, double d3, double d4, double d5, double d6,
                  double d7, double d8, struct FI s, int tail);
double theirs_dd(double d1, double d2, double d3, double d4, double d5, double d6,
                 double d7, double d8, struct DD s, int tail);
double theirs_d1(double d1, double d2, double d3, double d4, double d5, double d6,
                 double d7, double d8, int x, struct D1 s, int tail);

double call_ff(void)
{ struct FF s; s.a = 1.0f; s.b = 2.0f; return theirs_ff(1, 2, 3, 4, 5, 6, 7, s, 9); }
double call_fi7(void)
{ struct FI s; s.f = 1.0f; s.i = 2; return theirs_fi7(1, 2, 3, 4, 5, 6, 7, s, 9); }
double call_fi8(void)
{ struct FI s; s.f = 1.0f; s.i = 2; return theirs_fi8(1, 2, 3, 4, 5, 6, 7, 8, s, 9); }
double call_dd(void)
{ struct DD s; s.a = 1.0; s.b = 2.0; return theirs_dd(1, 2, 3, 4, 5, 6, 7, 8, s, 9); }
double call_d1(void)
{ struct D1 s; s.a = 1.0; return theirs_d1(1, 2, 3, 4, 5, 6, 7, 8, 5, s, 9); }
