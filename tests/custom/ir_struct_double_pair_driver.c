struct DD { double a; double b; };

double ours_dd(struct DD s);
double ours_dd2(struct DD s, struct DD t, double x);
struct DD ours_mkdd(double x);
double call_0(void);
double call_1(void);
double call_2(void);

double theirs_dd(struct DD s)
{ return s.a * 10.0 + s.b; }

double theirs_dd2(struct DD s, struct DD t, double x)
{ return s.a * 1000.0 + s.b * 100.0 + t.a * 10.0 + t.b + x * 0.5; }

struct DD theirs_mkdd(double x)
{ struct DD s; s.a = x; s.b = x + 1.0; return s; }

int main(void)
{
    { struct DD s; s.a = 1.5; s.b = 2.5; if ((ours_dd(s)) != (17.5)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (17.5)) return 2;   /* our caller, GCC callee */
    { struct DD s, t; s.a = 1.0; s.b = 2.0; t.a = 3.0; t.b = 4.0; if ((ours_dd2(s, t, 8.0)) != (1238)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (1238)) return 4;   /* our caller, GCC callee */
    { struct DD v; if (((v = ours_mkdd(2.5), v.a * 10.0 + v.b)) != (28.5)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (28.5)) return 6;   /* our caller, GCC callee */
    return 0;
}
