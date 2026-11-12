/* All eight a registers are used, so small structs (8, 4 and 3 bytes) go on the
   stack in 4-byte slots, in order, and the last int follows them.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct P8 { int a; int b; };
struct S1 { int a; };
struct S3 { char a; char b; char c; };

int ours_st(int a, int b, int c, int d, int e, int f, int g, int h, struct P8 s, struct S1 t, struct S3 u, int tail)
{ return a + h + s.a * 1000 + s.b * 100 + t.a * 10 + u.a + u.b + u.c + tail * 10000; }

int theirs_st(int a, int b, int c, int d, int e, int f, int g, int h, struct P8 s, struct S1 t, struct S3 u, int tail);

double call_0(void)
{
    struct P8 s; struct S1 t; struct S3 u; s.a = 1; s.b = 2; t.a = 3; u.a = 1; u.b = 2; u.c = 3;
    return theirs_st(10, 0, 0, 0, 0, 0, 0, 20, s, t, u, 5);
}
