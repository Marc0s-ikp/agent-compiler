struct S1 { int a; };
struct S2 { short a; short b; };
struct S3 { char a; char b; char c; };

int ours_f1(struct S1 s, int n);
int ours_f2(struct S2 s, int n);
int ours_f3(struct S3 s, int n);
struct S1 ours_m1(int x);
struct S2 ours_m2(int x);
struct S3 ours_m3(int x);
double call_0(void);
double call_1(void);
double call_2(void);
double call_3(void);
double call_4(void);
double call_5(void);

int theirs_f1(struct S1 s, int n)
{ return s.a * 10 + n; }

int theirs_f2(struct S2 s, int n)
{ return s.a * 100 + s.b * 10 + n; }

int theirs_f3(struct S3 s, int n)
{ return s.a * 1000 + s.b * 100 + s.c * 10 + n; }

struct S1 theirs_m1(int x)
{ struct S1 s; s.a = x * 2; return s; }

struct S2 theirs_m2(int x)
{ struct S2 s; s.a = x; s.b = -x; return s; }

struct S3 theirs_m3(int x)
{ struct S3 s; s.a = x; s.b = x + 1; s.c = x + 2; return s; }

int main(void)
{
    { struct S1 s; s.a = 4; if ((ours_f1(s, 3)) != (43)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (43)) return 2;   /* our caller, GCC callee */
    { struct S2 s; s.a = 7; s.b = -3; if ((ours_f2(s, 1)) != (671)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (671)) return 4;   /* our caller, GCC callee */
    { struct S3 s; s.a = 1; s.b = 2; s.c = 3; if ((ours_f3(s, 4)) != (1234)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (1234)) return 6;   /* our caller, GCC callee */
    { struct S1 v; if (((v = ours_m1(21), v.a)) != (42)) return 7; }   /* GCC caller, our callee */
    if (call_3() != (42)) return 8;   /* our caller, GCC callee */
    { struct S2 v; if (((v = ours_m2(5), v.a * 10 + v.b)) != (45)) return 9; }   /* GCC caller, our callee */
    if (call_4() != (45)) return 10;   /* our caller, GCC callee */
    { struct S3 v; if (((v = ours_m3(6), v.a * 100 + v.b * 10 + v.c)) != (678)) return 11; }   /* GCC caller, our callee */
    if (call_5() != (678)) return 12;   /* our caller, GCC callee */
    return 0;
}
