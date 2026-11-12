struct Big { int a; int b; int c; };
struct Wide { int a; int b; int c; int d; };
struct P8 { int a; int b; };

struct Big ours_mkb(int x, int y);
struct Wide ours_mkw(int x, double d, struct P8 p);
struct Big ours_id(struct Big b);
struct Big ours_many(int a, int b, int c, int d, int e, int f, int g, int h);
double call_0(void);
double call_1(void);
double call_2(void);
double call_3(void);

struct Big theirs_mkb(int x, int y)
{ struct Big s; s.a = x; s.b = y; s.c = x + y; return s; }

struct Wide theirs_mkw(int x, double d, struct P8 p)
{ struct Wide s; s.a = x; s.b = p.a; s.c = p.b; s.d = (int)(d * 2.0); return s; }

struct Big theirs_id(struct Big b)
{ b.c = b.c + 1; return b; }

struct Big theirs_many(int a, int b, int c, int d, int e, int f, int g, int h)
{ struct Big s; s.a = a + b; s.b = g; s.c = h; return s; }

int main(void)
{
    { struct Big v; if (((v = ours_mkb(10, 20), v.a * 10000 + v.b * 100 + v.c)) != (102030)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (102030)) return 2;   /* our caller, GCC callee */
    { struct Wide v; struct P8 p; p.a = 6; p.b = 7; if (((v = ours_mkw(5, 4.5, p), v.a * 1000 + v.b * 100 + v.c * 10 + v.d)) != (5679)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (5679)) return 4;   /* our caller, GCC callee */
    { struct Big v, b; b.a = 1; b.b = 2; b.c = 3; if (((v = ours_id(b), v.a * 100 + v.b * 10 + v.c + b.c * 1000)) != (3124)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (3124)) return 6;   /* our caller, GCC callee */
    { struct Big v; if (((v = ours_many(1, 2, 3, 4, 5, 6, 7, 8), v.a * 10000 + v.b * 100 + v.c)) != (30708)) return 7; }   /* GCC caller, our callee */
    if (call_3() != (30708)) return 8;   /* our caller, GCC callee */
    return 0;
}
