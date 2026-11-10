struct FF { float a; float b; };
struct FI { float f; int i; };
struct DD { double a; double b; };
struct D1 { double a; };

double ours_ff(double d1, double d2, double d3, double d4, double d5, double d6,
               double d7, struct FF s, int tail);
double ours_fi7(double d1, double d2, double d3, double d4, double d5, double d6,
                double d7, struct FI s, int tail);
double ours_fi8(double d1, double d2, double d3, double d4, double d5, double d6,
                double d7, double d8, struct FI s, int tail);
double ours_dd(double d1, double d2, double d3, double d4, double d5, double d6,
               double d7, double d8, struct DD s, int tail);
double ours_d1(double d1, double d2, double d3, double d4, double d5, double d6,
               double d7, double d8, int x, struct D1 s, int tail);
double call_ff(void);
double call_fi7(void);
double call_fi8(void);
double call_dd(void);
double call_d1(void);

double theirs_ff(double d1, double d2, double d3, double d4, double d5, double d6,
               double d7, struct FF s, int tail)
{ return d1 + d7 + s.a * 100.0 + s.b * 10.0 + tail; }
double theirs_fi7(double d1, double d2, double d3, double d4, double d5, double d6,
                double d7, struct FI s, int tail)
{ return d1 + d7 + s.f * 100.0 + s.i * 10.0 + tail; }
double theirs_fi8(double d1, double d2, double d3, double d4, double d5, double d6,
                double d7, double d8, struct FI s, int tail)
{ return d1 + d8 + s.f * 100.0 + s.i * 10.0 + tail; }
double theirs_dd(double d1, double d2, double d3, double d4, double d5, double d6,
               double d7, double d8, struct DD s, int tail)
{ return d1 + d8 + s.a * 100.0 + s.b * 10.0 + tail; }
double theirs_d1(double d1, double d2, double d3, double d4, double d5, double d6,
               double d7, double d8, int x, struct D1 s, int tail)
{ return d1 + d8 + x + s.a * 100.0 + tail; }

int main()
{
    struct FF ff; struct FI fi; struct DD dd; struct D1 d1;
    ff.a = 1.0f; ff.b = 2.0f; fi.f = 1.0f; fi.i = 2; dd.a = 1.0; dd.b = 2.0; d1.a = 1.0;
    if (ours_ff(1, 2, 3, 4, 5, 6, 7, ff, 9) != 137.0) return 1;
    if (ours_fi7(1, 2, 3, 4, 5, 6, 7, fi, 9) != 137.0) return 2;
    if (ours_fi8(1, 2, 3, 4, 5, 6, 7, 8, fi, 9) != 138.0) return 3;
    if (ours_dd(1, 2, 3, 4, 5, 6, 7, 8, dd, 9) != 138.0) return 4;
    if (ours_d1(1, 2, 3, 4, 5, 6, 7, 8, 5, d1, 9) != 123.0) return 5;
    if (call_ff() != 137.0) return 6;
    if (call_fi7() != 137.0) return 7;
    if (call_fi8() != 138.0) return 8;
    if (call_dd() != 138.0) return 9;
    if (call_d1() != 123.0) return 10;
    return 0;
}
