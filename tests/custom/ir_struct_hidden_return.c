/* A struct over 8 bytes comes back through a hidden pointer the caller passes in
   a0, so every declared argument moves up one register (the 8th int goes on the
   stack). The callee returns a struct it was given by reference, too.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct Big { int a; int b; int c; };
struct Wide { int a; int b; int c; int d; };
struct P8 { int a; int b; };

struct Big ours_mkb(int x, int y)
{ struct Big s; s.a = x; s.b = y; s.c = x + y; return s; }

struct Wide ours_mkw(int x, double d, struct P8 p)
{ struct Wide s; s.a = x; s.b = p.a; s.c = p.b; s.d = (int)(d * 2.0); return s; }

struct Big ours_id(struct Big b)
{ b.c = b.c + 1; return b; }

struct Big ours_many(int a, int b, int c, int d, int e, int f, int g, int h)
{ struct Big s; s.a = a + b; s.b = g; s.c = h; return s; }

struct Big theirs_mkb(int x, int y);
struct Wide theirs_mkw(int x, double d, struct P8 p);
struct Big theirs_id(struct Big b);
struct Big theirs_many(int a, int b, int c, int d, int e, int f, int g, int h);

double call_0(void)
{
    struct Big v;
    return (v = theirs_mkb(10, 20), v.a * 10000 + v.b * 100 + v.c);
}

double call_1(void)
{
    struct Wide v; struct P8 p; p.a = 6; p.b = 7;
    return (v = theirs_mkw(5, 4.5, p), v.a * 1000 + v.b * 100 + v.c * 10 + v.d);
}

double call_2(void)
{
    struct Big v, b; b.a = 1; b.b = 2; b.c = 3;
    return (v = theirs_id(b), v.a * 100 + v.b * 10 + v.c + b.c * 1000);
}

double call_3(void)
{
    struct Big v;
    return (v = theirs_many(1, 2, 3, 4, 5, 6, 7, 8), v.a * 10000 + v.b * 100 + v.c);
}
