struct C3 { char a; char b; char c; };
struct C5 { char c[5]; };
struct C7 { char c[7]; };
struct FP { float f; void *p; };
struct FU { float f; union { int i; float g; } u; };
struct Big { int v[40]; };
int theirs_c3(struct C3 s, int y) { return s.a * 1000 + s.b * 100 + s.c * 10 + y; }
int theirs_c5(int x, struct C5 s)
{ return x + s.c[0] + s.c[1] * 2 + s.c[2] * 3 + s.c[3] * 4 + s.c[4] * 5; }
int theirs_c7(int a, int b, int c, int d, int e, int f, struct C7 s, int y)
{ return a + b + c + d + e + f + s.c[0] + s.c[3] * 2 + s.c[6] * 3 + y; }
int theirs_c7_split(int a, int b, int c, int d, int e, int f, int g, struct C7 s, int y)
{ return a + g + s.c[0] + s.c[3] * 2 + s.c[6] * 3 + y * 100; }
int theirs_c7_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct C7 s, int y)
{ return a + h + s.c[0] + s.c[3] * 2 + s.c[6] * 3 + y * 100; }
struct C3 theirs_mk3(int x) { struct C3 s; s.a = x; s.b = x + 1; s.c = x + 2; return s; }
struct C5 theirs_mk5(int x) { struct C5 s; int i; for (i = 0; i < 5; i++) s.c[i] = x + i; return s; }
struct C7 theirs_mk7(int x) { struct C7 s; int i; for (i = 0; i < 7; i++) s.c[i] = x * 2 + i; return s; }
/* a pointer or a union stops a struct from using the floating registers */
float theirs_fp(struct FP s, int n) { return s.f * n + (s.p != 0); }
struct FP theirs_mkfp(float f, void *p) { struct FP s; s.f = f; s.p = p; return s; }
double theirs_fu(struct FU s, double d) { return s.f + s.u.i + d; }
struct FU theirs_mkfu(float f, int i) { struct FU s; s.f = f; s.u.i = i; return s; }
/* 160 bytes: copied by a loop, passed by reference, returned through a pointer */
int theirs_big(struct Big b) { int i, s; s = 0; for (i = 0; i < 40; i++) s += b.v[i] * (i + 1); b.v[0] = 999; return s; }
struct Big theirs_mkbig(int x) { struct Big b; int i; for (i = 0; i < 40; i++) b.v[i] = x + i; return b; }

extern int ours_c3(struct C3 s, int y);
extern int ours_c5(int x, struct C5 s);
extern int ours_c7(int a, int b, int c, int d, int e, int f, struct C7 s, int y);
extern int ours_c7_split(int a, int b, int c, int d, int e, int f, int g, struct C7 s, int y);
extern int ours_c7_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct C7 s, int y);
extern struct C3 ours_mk3(int x);
extern struct C5 ours_mk5(int x);
extern struct C7 ours_mk7(int x);
extern float ours_fp(struct FP s, int n);
extern struct FP ours_mkfp(float f, void *p);
extern double ours_fu(struct FU s, double d);
extern struct FU ours_mkfu(float f, int i);
extern int ours_big(struct Big b);
extern struct Big ours_mkbig(int x);
int call_c3(void); int call_c5(void); int call_c7(void); int call_c7_split(void); int call_c7_stack(void);
int call_mk3(void); int call_mk5(void); int call_mk7(void); float call_fp(void); float call_mkfp(void);
double call_fu(void); double call_mkfu(void); int call_big(void); int call_mkbig(void);

static struct C3 c3(void) { struct C3 s; s.a = 1; s.b = 2; s.c = 3; return s; }
static struct C5 c5(void) { struct C5 s; int i; for (i = 0; i < 5; i++) s.c[i] = i + 1; return s; }
static struct C7 c7(void) { struct C7 s; int i; for (i = 0; i < 7; i++) s.c[i] = i + 1; return s; }

int main()
{
    struct C3 a3, b3; struct C5 a5, b5; struct C7 a7, b7;
    struct FP fp; struct FU fu; struct Big big, bb; int x, i;
    if (ours_c3(c3(), 4) != theirs_c3(c3(), 4)) return 1;
    if (ours_c5(7, c5()) != theirs_c5(7, c5())) return 2;
    if (ours_c7(1, 2, 3, 4, 5, 6, c7(), 8) != theirs_c7(1, 2, 3, 4, 5, 6, c7(), 8)) return 3;
    if (ours_c7_split(1, 2, 3, 4, 5, 6, 7, c7(), 8) != theirs_c7_split(1, 2, 3, 4, 5, 6, 7, c7(), 8)) return 4;
    if (ours_c7_stack(1, 2, 3, 4, 5, 6, 7, 8, c7(), 9) != theirs_c7_stack(1, 2, 3, 4, 5, 6, 7, 8, c7(), 9)) return 5;
    a3 = ours_mk3(10); b3 = theirs_mk3(10);
    if (a3.a != b3.a || a3.b != b3.b || a3.c != b3.c) return 6;
    a5 = ours_mk5(10); b5 = theirs_mk5(10);
    for (i = 0; i < 5; i++) if (a5.c[i] != b5.c[i]) return 7;
    a7 = ours_mk7(10); b7 = theirs_mk7(10);
    for (i = 0; i < 7; i++) if (a7.c[i] != b7.c[i]) return 8;
    fp.f = 2.5f; fp.p = &x;
    if (ours_fp(fp, 3) != theirs_fp(fp, 3)) return 9;
    fp = ours_mkfp(1.5f, &x);
    if (fp.f != 1.5f || fp.p != &x) return 10;
    fu.f = 1.5f; fu.u.i = 4;
    if (ours_fu(fu, 0.25) != theirs_fu(fu, 0.25)) return 11;
    fu = ours_mkfu(1.5f, 9);
    if (fu.f != 1.5f || fu.u.i != 9) return 12;
    for (i = 0; i < 40; i++) big.v[i] = i;
    if (ours_big(big) != theirs_big(big)) return 13;
    if (big.v[0] != 0) return 14;
    bb = ours_mkbig(3);
    for (i = 0; i < 40; i++) if (bb.v[i] != 3 + i) return 15;
    if (call_c3() != theirs_c3(c3(), 4)) return 21;
    if (call_c5() != theirs_c5(7, c5())) return 22;
    if (call_c7() != theirs_c7(1, 2, 3, 4, 5, 6, c7(), 8)) return 23;
    if (call_c7_split() != theirs_c7_split(1, 2, 3, 4, 5, 6, 7, c7(), 8)) return 24;
    if (call_c7_stack() != theirs_c7_stack(1, 2, 3, 4, 5, 6, 7, 8, c7(), 9)) return 25;
    if (call_mk3() != 1 * 10 + 11 * 100 + 12 * 10000) return 26;
    if (call_mk5() != 10 * 81 + 11 * 27 + 12 * 9 + 13 * 3 + 14) return 27;
    if (call_mk7() != 20 * 729 + 21 * 243 + 22 * 81 + 23 * 27 + 24 * 9 + 25 * 3 + 26) return 28;
    if (call_fp() != 8.5f) return 29;
    if (call_mkfp() != 2.5f) return 30;
    if (call_fu() != 5.75) return 31;
    if (call_mkfu() != 10.5) return 32;
    { struct Big g; int s = 0; for (i = 0; i < 40; i++) { g.v[i] = i; s += i * (i + 1); } if (call_big() != s * 10 + 1) return 33; }
    { int s = 0; for (i = 0; i < 40; i++) s += (3 + i) * (i % 5 + 1); if (call_mkbig() != s) return 34; }
    return 0;
}
