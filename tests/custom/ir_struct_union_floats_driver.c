union UF { float a; float b; };
union UD { double d; float f; };

double ours_uf(double d1, union UF u, double d2);
double ours_ud(union UD u, int n);
union UF ours_mku(float x);
union UD ours_mkd(double x);
double call_0(void);
double call_1(void);
double call_2(void);
double call_3(void);

double theirs_uf(double d1, union UF u, double d2)
{ return d1 * 100.0 + u.a * 10.0 + d2; }

double theirs_ud(union UD u, int n)
{ return u.d * 10.0 + n; }

union UF theirs_mku(float x)
{ union UF u; u.a = x; return u; }

union UD theirs_mkd(double x)
{ union UD u; u.d = x; return u; }

int main(void)
{
    { union UF u; u.a = 1.5f; if ((ours_uf(2.0, u, 3.0)) != (218)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (218)) return 2;   /* our caller, GCC callee */
    { union UD u; u.d = 2.5; if ((ours_ud(u, 4)) != (29)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (29)) return 4;   /* our caller, GCC callee */
    { union UF v; if (((v = ours_mku(2.5f), v.b)) != (2.5)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (2.5)) return 6;   /* our caller, GCC callee */
    { union UD v; if (((v = ours_mkd(2.5), v.d)) != (2.5)) return 7; }   /* GCC caller, our callee */
    if (call_3() != (2.5)) return 8;   /* our caller, GCC callee */
    return 0;
}
