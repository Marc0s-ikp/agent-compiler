/* Edge cases of the struct ABI: sizes that are not a multiple of 4 (partial
   words), a pointer or a union inside an otherwise floating struct (integer
   convention), and a 160-byte struct (copy loop, by reference, hidden
   return pointer). Written once; the driver holds the same code for GCC. */
struct C3 { char a; char b; char c; };
struct C5 { char c[5]; };
struct C7 { char c[7]; };
struct FP { float f; void *p; };
struct FU { float f; union { int i; float g; } u; };
struct Big { int v[40]; };
int ours_c3(struct C3 s, int y) { return s.a * 1000 + s.b * 100 + s.c * 10 + y; }
int ours_c5(int x, struct C5 s)
{ return x + s.c[0] + s.c[1] * 2 + s.c[2] * 3 + s.c[3] * 4 + s.c[4] * 5; }
int ours_c7(int a, int b, int c, int d, int e, int f, struct C7 s, int y)
{ return a + b + c + d + e + f + s.c[0] + s.c[3] * 2 + s.c[6] * 3 + y; }
int ours_c7_split(int a, int b, int c, int d, int e, int f, int g, struct C7 s, int y)
{ return a + g + s.c[0] + s.c[3] * 2 + s.c[6] * 3 + y * 100; }
int ours_c7_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct C7 s, int y)
{ return a + h + s.c[0] + s.c[3] * 2 + s.c[6] * 3 + y * 100; }
struct C3 ours_mk3(int x) { struct C3 s; s.a = x; s.b = x + 1; s.c = x + 2; return s; }
struct C5 ours_mk5(int x) { struct C5 s; int i; for (i = 0; i < 5; i++) s.c[i] = x + i; return s; }
struct C7 ours_mk7(int x) { struct C7 s; int i; for (i = 0; i < 7; i++) s.c[i] = x * 2 + i; return s; }
/* a pointer or a union stops a struct from using the floating registers */
float ours_fp(struct FP s, int n) { return s.f * n + (s.p != 0); }
struct FP ours_mkfp(float f, void *p) { struct FP s; s.f = f; s.p = p; return s; }
double ours_fu(struct FU s, double d) { return s.f + s.u.i + d; }
struct FU ours_mkfu(float f, int i) { struct FU s; s.f = f; s.u.i = i; return s; }
/* 160 bytes: copied by a loop, passed by reference, returned through a pointer */
int ours_big(struct Big b) { int i, s; s = 0; for (i = 0; i < 40; i++) s += b.v[i] * (i + 1); b.v[0] = 999; return s; }
struct Big ours_mkbig(int x) { struct Big b; int i; for (i = 0; i < 40; i++) b.v[i] = x + i; return b; }

extern int theirs_c3(struct C3 s, int y);
extern int theirs_c5(int x, struct C5 s);
extern int theirs_c7(int a, int b, int c, int d, int e, int f, struct C7 s, int y);
extern int theirs_c7_split(int a, int b, int c, int d, int e, int f, int g, struct C7 s, int y);
extern int theirs_c7_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct C7 s, int y);
extern struct C3 theirs_mk3(int x);
extern struct C5 theirs_mk5(int x);
extern struct C7 theirs_mk7(int x);
extern float theirs_fp(struct FP s, int n);
extern struct FP theirs_mkfp(float f, void *p);
extern double theirs_fu(struct FU s, double d);
extern struct FU theirs_mkfu(float f, int i);
extern int theirs_big(struct Big b);
extern struct Big theirs_mkbig(int x);

int call_c3(void) { struct C3 s; s.a = 1; s.b = 2; s.c = 3; return theirs_c3(s, 4); }
int call_c5(void) { struct C5 s; int i; for (i = 0; i < 5; i++) s.c[i] = i + 1; return theirs_c5(7, s); }
int call_c7(void) { struct C7 s; int i; for (i = 0; i < 7; i++) s.c[i] = i + 1; return theirs_c7(1, 2, 3, 4, 5, 6, s, 8); }
int call_c7_split(void) { struct C7 s; int i; for (i = 0; i < 7; i++) s.c[i] = i + 1; return theirs_c7_split(1, 2, 3, 4, 5, 6, 7, s, 8); }
int call_c7_stack(void) { struct C7 s; int i; for (i = 0; i < 7; i++) s.c[i] = i + 1; return theirs_c7_stack(1, 2, 3, 4, 5, 6, 7, 8, s, 9); }
int call_mk3(void) { struct C3 s; s = theirs_mk3(10); return s.a + s.b * 100 + s.c * 10000; }
int call_mk5(void) { struct C5 s; int i, r; s = theirs_mk5(10); r = 0; for (i = 0; i < 5; i++) r = r * 3 + s.c[i]; return r; }
int call_mk7(void) { struct C7 s; int i, r; s = theirs_mk7(10); r = 0; for (i = 0; i < 7; i++) r = r * 3 + s.c[i]; return r; }
float call_fp(void) { struct FP s; int x; s.f = 2.5f; s.p = &x; return theirs_fp(s, 3); }
float call_mkfp(void) { struct FP s; int x; s = theirs_mkfp(1.5f, &x); return s.f + (s.p == &x); }
double call_fu(void) { struct FU s; s.f = 1.5f; s.u.i = 4; return theirs_fu(s, 0.25); }
double call_mkfu(void) { struct FU s; s = theirs_mkfu(1.5f, 9); return s.f + s.u.i; }
int call_big(void) { struct Big b; int i; for (i = 0; i < 40; i++) b.v[i] = i; i = theirs_big(b); return i * 10 + (b.v[0] == 0); }
int call_mkbig(void) { struct Big b; int i, s; b = theirs_mkbig(3); s = 0; for (i = 0; i < 40; i++) s += b.v[i] * (i % 5 + 1); return s; }
