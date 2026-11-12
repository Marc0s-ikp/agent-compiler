struct P8 { int a; int b; };
struct S1 { int a; };
struct S3 { char a; char b; char c; };

int ours_st(int a, int b, int c, int d, int e, int f, int g, int h, struct P8 s, struct S1 t, struct S3 u, int tail);
double call_0(void);

int theirs_st(int a, int b, int c, int d, int e, int f, int g, int h, struct P8 s, struct S1 t, struct S3 u, int tail)
{ return a + h + s.a * 1000 + s.b * 100 + t.a * 10 + u.a + u.b + u.c + tail * 10000; }

int main(void)
{
    { struct P8 s; struct S1 t; struct S3 u; s.a = 1; s.b = 2; t.a = 3; u.a = 1; u.b = 2; u.c = 3; if ((ours_st(10, 0, 0, 0, 0, 0, 0, 20, s, t, u, 5)) != (51266)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (51266)) return 2;   /* our caller, GCC callee */
    return 0;
}
