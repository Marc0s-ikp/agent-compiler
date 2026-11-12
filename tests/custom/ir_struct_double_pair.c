/* {double,double} (16 bytes) travels in fa0 and fa1, not by reference; two of
   them use fa0-fa3, and the double after them gets fa4. Also as a return value.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct DD { double a; double b; };

double ours_dd(struct DD s)
{ return s.a * 10.0 + s.b; }

double ours_dd2(struct DD s, struct DD t, double x)
{ return s.a * 1000.0 + s.b * 100.0 + t.a * 10.0 + t.b + x * 0.5; }

struct DD ours_mkdd(double x)
{ struct DD s; s.a = x; s.b = x + 1.0; return s; }

double theirs_dd(struct DD s);
double theirs_dd2(struct DD s, struct DD t, double x);
struct DD theirs_mkdd(double x);

double call_0(void)
{
    struct DD s; s.a = 1.5; s.b = 2.5;
    return theirs_dd(s);
}

double call_1(void)
{
    struct DD s, t; s.a = 1.0; s.b = 2.0; t.a = 3.0; t.b = 4.0;
    return theirs_dd2(s, t, 8.0);
}

double call_2(void)
{
    struct DD v;
    return (v = theirs_mkdd(2.5), v.a * 10.0 + v.b);
}
