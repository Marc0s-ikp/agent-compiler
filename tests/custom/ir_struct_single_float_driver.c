struct F { float x; };
struct D { double x; };

float ours_pf(struct F s, int n);
double ours_pd(struct D s, int n);
struct F ours_mf(float v);
struct D ours_md(double v);
double call_0(void);
double call_1(void);
double call_2(void);
double call_3(void);

float theirs_pf(struct F s, int n)
{ return s.x * n; }

double theirs_pd(struct D s, int n)
{ return s.x * n; }

struct F theirs_mf(float v)
{ struct F s; s.x = v + 1.0f; return s; }

struct D theirs_md(double v)
{ struct D s; s.x = v + 1.0; return s; }

int main(void)
{
    { struct F s; s.x = 2.5f; if ((ours_pf(s, 3)) != (7.5)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (7.5)) return 2;   /* our caller, GCC callee */
    { struct D s; s.x = 2.5; if ((ours_pd(s, 3)) != (7.5)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (7.5)) return 4;   /* our caller, GCC callee */
    { struct F v; if (((v = ours_mf(4.0f), v.x)) != (5)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (5)) return 6;   /* our caller, GCC callee */
    { struct D v; if (((v = ours_md(4.0), v.x)) != (5)) return 7; }   /* GCC caller, our callee */
    if (call_3() != (5)) return 8;   /* our caller, GCC callee */
    return 0;
}
