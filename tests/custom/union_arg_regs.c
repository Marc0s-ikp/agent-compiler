/* Union placement in the argument list: a float union takes an a register
   (not fa), so the double that follows still gets fa0. A 12-byte union goes
   by reference like a big struct, and the callee's writes do not reach the
   caller's object. */
union UF { float f; int i; };
union Big { double d; char c[12]; };

double ours_mix(int a, union UF u, double d, int b)
{ return a * 1000.0 + u.i * 100.0 + d * 10.0 + b; }

int ours_big(union Big u)
{
    u.c[0] = 99;
    return u.c[0] + u.c[11];
}

double theirs_mix(int a, union UF u, double d, int b);
int theirs_big(union Big u);

double call_mix(void) { union UF u; u.i = 2; return theirs_mix(1, u, 3.0, 4); }
int call_big(void)
{
    union Big u;
    int i, r;
    for (i = 0; i < 12; i++) u.c[i] = i;
    r = theirs_big(u);
    return r * 100 + u.c[0];      /* callee's copy was modified, ours not */
}
