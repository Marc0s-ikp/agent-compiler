struct P8 { int a; int b; };
struct C7 { char c[7]; };

int ours_sp8(int a, int b, int c, int d, int e, int f, int g, struct P8 s, int tail);
int ours_sp7(int a, int b, int c, int d, int e, int f, int g, struct C7 s, int tail);
double call_0(void);
double call_1(void);

int theirs_sp8(int a, int b, int c, int d, int e, int f, int g, struct P8 s, int tail)
{ return a + b + c + d + e + f + g + s.a * 100 + s.b * 10 + tail; }

int theirs_sp7(int a, int b, int c, int d, int e, int f, int g, struct C7 s, int tail)
{ return g + s.c[0] * 100 + s.c[3] * 10 + s.c[6] + tail; }

int main(void)
{
    { struct P8 s; s.a = 4; s.b = 5; if ((ours_sp8(1, 2, 3, 4, 5, 6, 7, s, 9)) != (487)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (487)) return 2;   /* our caller, GCC callee */
    { struct C7 s; int i; for (i = 0; i < 7; i++) s.c[i] = i + 1; if ((ours_sp7(1, 2, 3, 4, 5, 6, 7, s, 9)) != (163)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (163)) return 4;   /* our caller, GCC callee */
    return 0;
}
