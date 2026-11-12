/* struct {float} and struct {double} travel in one fa register (the field,
   flattened), as arguments and as return values.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct F { float x; };
struct D { double x; };

float ours_pf(struct F s, int n)
{ return s.x * n; }

double ours_pd(struct D s, int n)
{ return s.x * n; }

struct F ours_mf(float v)
{ struct F s; s.x = v + 1.0f; return s; }

struct D ours_md(double v)
{ struct D s; s.x = v + 1.0; return s; }

float theirs_pf(struct F s, int n);
double theirs_pd(struct D s, int n);
struct F theirs_mf(float v);
struct D theirs_md(double v);

double call_0(void)
{
    struct F s; s.x = 2.5f;
    return theirs_pf(s, 3);
}

double call_1(void)
{
    struct D s; s.x = 2.5;
    return theirs_pd(s, 3);
}

double call_2(void)
{
    struct F v;
    return (v = theirs_mf(4.0f), v.x);
}

double call_3(void)
{
    struct D v;
    return (v = theirs_md(4.0), v.x);
}
