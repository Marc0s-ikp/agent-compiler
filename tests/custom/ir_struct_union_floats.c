/* A union never uses the floating convention, even when all its members are
   floats: {float a; float b;} travels in an a register (the double after it
   still gets fa0) and comes back in a0, not fa0. An 8-byte union of double and
   float uses two a registers.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
union UF { float a; float b; };
union UD { double d; float f; };

double ours_uf(double d1, union UF u, double d2)
{ return d1 * 100.0 + u.a * 10.0 + d2; }

double ours_ud(union UD u, int n)
{ return u.d * 10.0 + n; }

union UF ours_mku(float x)
{ union UF u; u.a = x; return u; }

union UD ours_mkd(double x)
{ union UD u; u.d = x; return u; }

double theirs_uf(double d1, union UF u, double d2);
double theirs_ud(union UD u, int n);
union UF theirs_mku(float x);
union UD theirs_mkd(double x);

double call_0(void)
{
    union UF u; u.a = 1.5f;
    return theirs_uf(2.0, u, 3.0);
}

double call_1(void)
{
    union UD u; u.d = 2.5;
    return theirs_ud(u, 4);
}

double call_2(void)
{
    union UF v;
    return (v = theirs_mku(2.5f), v.b);
}

double call_3(void)
{
    union UD v;
    return (v = theirs_mkd(2.5), v.d);
}
