struct Big { int a; int b; int c; };

int ours_clobber(struct Big s);
int ours_middle(int x, struct Big s, int y);
int ours_ninth(int a, int b, int c, int d, int e, int f, int g, int h, struct Big s, int tail);
int ours_twice(struct Big s, struct Big t);
double call_0(void);
double call_1(void);
double call_2(void);
double call_3(void);

int theirs_clobber(struct Big s)
{ s.a = 100; s.b = 200; s.c = 300; return s.a + s.b + s.c; }

int theirs_middle(int x, struct Big s, int y)
{ return x * 1000 + s.a * 100 + s.b * 10 + s.c + y; }

int theirs_ninth(int a, int b, int c, int d, int e, int f, int g, int h, struct Big s, int tail)
{ s.a = s.a + 1; return a + h + s.a * 100 + s.b * 10 + s.c + tail * 1000; }

int theirs_twice(struct Big s, struct Big t)
{ t.a = s.c; return s.a * 100 + s.b * 10 + s.c + t.a * 1000 + t.b * 10000; }

int main(void)
{
    { struct Big s; s.a = 1; s.b = 2; s.c = 3; if ((ours_clobber(s) * 1000 + s.a * 100 + s.b * 10 + s.c) != (600123)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (600123)) return 2;   /* our caller, GCC callee */
    { struct Big s; s.a = 1; s.b = 2; s.c = 3; if ((ours_middle(7, s, 9)) != (7132)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (7132)) return 4;   /* our caller, GCC callee */
    { struct Big s; s.a = 1; s.b = 2; s.c = 3; if ((ours_ninth(1, 2, 3, 4, 5, 6, 7, 8, s, 9) * 10 + s.a) != (92321)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (92321)) return 6;   /* our caller, GCC callee */
    { struct Big s, t; s.a = 1; s.b = 2; s.c = 3; t.a = 4; t.b = 5; t.c = 6; if ((ours_twice(s, t) + t.a * 100000) != (453123)) return 7; }   /* GCC caller, our callee */
    if (call_3() != (453123)) return 8;   /* our caller, GCC callee */
    return 0;
}
