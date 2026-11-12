struct P8 { int a; int b; };
struct P5 { char c[5]; };
struct P6 { short a; short b; short c; };
struct P8m { char a; int b; };

int ours_g8(struct P8 s, int n);
int ours_g5(struct P5 s, int n);
int ours_g6(struct P6 s, int n);
int ours_g8m(struct P8m s, int n);
struct P8 ours_n8(int x);
struct P5 ours_n5(int x);
struct P6 ours_n6(int x);
struct P8m ours_n8m(int x);
double call_0(void);
double call_1(void);
double call_2(void);
double call_3(void);
double call_4(void);
double call_5(void);
double call_6(void);
double call_7(void);

int theirs_g8(struct P8 s, int n)
{ return s.a * 100 + s.b * 10 + n; }

int theirs_g5(struct P5 s, int n)
{ return s.c[0] * 10000 + s.c[1] * 1000 + s.c[2] * 100 + s.c[3] * 10 + s.c[4] + n; }

int theirs_g6(struct P6 s, int n)
{ return s.a * 100 + s.b * 10 + s.c + n; }

int theirs_g8m(struct P8m s, int n)
{ return s.a * 100 + s.b + n; }

struct P8 theirs_n8(int x)
{ struct P8 s; s.a = x; s.b = x * 2; return s; }

struct P5 theirs_n5(int x)
{ struct P5 s; int i; for (i = 0; i < 5; i++) s.c[i] = x + i; return s; }

struct P6 theirs_n6(int x)
{ struct P6 s; s.a = x; s.b = -x; s.c = x * 3; return s; }

struct P8m theirs_n8m(int x)
{ struct P8m s; s.a = x; s.b = x * 1000; return s; }

int main(void)
{
    { struct P8 s; s.a = 1; s.b = 2; if ((ours_g8(s, 3)) != (123)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (123)) return 2;   /* our caller, GCC callee */
    { struct P5 s; s.c[0] = 1; s.c[1] = 2; s.c[2] = 3; s.c[3] = 4; s.c[4] = 5; if ((ours_g5(s, 6)) != (12351)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (12351)) return 4;   /* our caller, GCC callee */
    { struct P6 s; s.a = 1; s.b = -2; s.c = 3; if ((ours_g6(s, 4)) != (87)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (87)) return 6;   /* our caller, GCC callee */
    { struct P8m s; s.a = 5; s.b = 70000; if ((ours_g8m(s, 1)) != (70501)) return 7; }   /* GCC caller, our callee */
    if (call_3() != (70501)) return 8;   /* our caller, GCC callee */
    { struct P8 v; if (((v = ours_n8(7), v.a * 100 + v.b)) != (714)) return 9; }   /* GCC caller, our callee */
    if (call_4() != (714)) return 10;   /* our caller, GCC callee */
    { struct P5 v; if (((v = ours_n5(10), v.c[0] * 10000 + v.c[2] * 100 + v.c[4])) != (101214)) return 11; }   /* GCC caller, our callee */
    if (call_5() != (101214)) return 12;   /* our caller, GCC callee */
    { struct P6 v; if (((v = ours_n6(4), v.a * 100 + v.b * 10 + v.c)) != (372)) return 13; }   /* GCC caller, our callee */
    if (call_6() != (372)) return 14;   /* our caller, GCC callee */
    { struct P8m v; if (((v = ours_n8m(9), v.a * 100000 + v.b)) != (909000)) return 15; }   /* GCC caller, our callee */
    if (call_7() != (909000)) return 16;   /* our caller, GCC callee */
    return 0;
}
