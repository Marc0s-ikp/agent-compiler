/* Structs with short members passed and returned by value (ilp32d). ours_* are
   compiled here, theirs_* by GCC; call_* make our caller talk to GCC callees. */
struct SF { short s; float f; };
struct FS { float f; short s; };
struct SD { short s; double d; };
struct SS { short a; short b; };
struct CS { signed char c; short s; };
struct SSS { short a; short b; short c; };
struct SI { short s; int i; };
struct USF { unsigned short u; float f; };
float ours_sf(struct SF x, int n) { return x.s * n + x.f; }
struct SF ours_mksf(short s, float f) { struct SF x; x.s = s; x.f = f; return x; }
float ours_fs(struct FS x) { return x.f * 2 + x.s; }
struct FS ours_mkfs(float f, short s) { struct FS x; x.f = f; x.s = s; return x; }
double ours_sd(struct SD x) { return x.d + x.s; }
struct SD ours_mksd(short s, double d) { struct SD x; x.s = s; x.d = d; return x; }
int ours_ss(struct SS x) { return x.a * 1000 + x.b; }
struct SS ours_mkss(short a, short b) { struct SS x; x.a = a; x.b = b; return x; }
int ours_cs(struct CS x) { return x.c * 1000 + x.s; }
struct CS ours_mkcs(signed char c, short s) { struct CS x; x.c = c; x.s = s; return x; }
int ours_sss(struct SSS x) { return x.a * 10000 + x.b * 100 + x.c; }
struct SSS ours_mksss(short a, short b, short c) { struct SSS x; x.a = a; x.b = b; x.c = c; return x; }
int ours_si(struct SI x) { return x.s * 1000 + x.i; }
struct SI ours_mksi(short s, int i) { struct SI x; x.s = s; x.i = i; return x; }
float ours_usf(struct USF x) { return x.u + x.f; }
struct USF ours_mkusf(unsigned short u, float f) { struct USF x; x.u = u; x.f = f; return x; }
/* the integer registers run out: both structs go on the stack */
float ours_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct SS x, struct SF y)
{ return a + h + x.a + x.b + y.s + y.f; }
float ours_split(int a, int b, int c, int d, int e, int f, int g, struct SS x, struct CS y)
{ return a + g + x.a + x.b + y.c + y.s; }
extern float theirs_sf(struct SF x, int n);
extern struct SF theirs_mksf(short s, float f);
extern float theirs_fs(struct FS x);
extern struct FS theirs_mkfs(float f, short s);
extern double theirs_sd(struct SD x);
extern struct SD theirs_mksd(short s, double d);
extern int theirs_ss(struct SS x);
extern struct SS theirs_mkss(short a, short b);
extern int theirs_cs(struct CS x);
extern struct CS theirs_mkcs(signed char c, short s);
extern int theirs_sss(struct SSS x);
extern struct SSS theirs_mksss(short a, short b, short c);
extern int theirs_si(struct SI x);
extern struct SI theirs_mksi(short s, int i);
extern float theirs_usf(struct USF x);
extern struct USF theirs_mkusf(unsigned short u, float f);
extern float theirs_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct SS x, struct SF y);
extern float theirs_split(int a, int b, int c, int d, int e, int f, int g, struct SS x, struct CS y);

int call_sf(void) { struct SF x = theirs_mksf(-4, 1.5f); return (int)(theirs_sf(x, 3) * 10); }
int call_fs(void) { struct FS x = theirs_mkfs(0.5f, -6); return (int)(theirs_fs(x) * 10); }
int call_sd(void) { struct SD x = theirs_mksd(-100, 0.25); return (int)(theirs_sd(x) * 100); }
int call_ss(void) { struct SS x = theirs_mkss(-3, -4); return theirs_ss(x); }
int call_cs(void) { struct CS x = theirs_mkcs(-5, -300); return theirs_cs(x); }
int call_sss(void) { struct SSS x = theirs_mksss(-1, -2, -3); return theirs_sss(x); }
int call_si(void) { struct SI x = theirs_mksi(-7, -8); return theirs_si(x); }
int call_usf(void) { struct USF x = theirs_mkusf(65535, 0.5f); return (int)(theirs_usf(x) * 2); }
int call_stack(void)
{
    struct SS x; struct SF y;
    x.a = -10; x.b = -20; y.s = -30; y.f = 4.0f;
    return (int)theirs_stack(1, 2, 3, 4, 5, 6, 7, 8, x, y);
}
int call_split(void)
{
    struct SS x; struct CS y;
    x.a = -10; x.b = -20; y.c = -3; y.s = -400;
    return (int)theirs_split(1, 2, 3, 4, 5, 6, 7, x, y);
}
/* struct values copied through memory and arrays keep the sign of every field */
int copy_chain(void)
{
    struct SF a[2]; struct SF b; struct SF c;
    a[0] = theirs_mksf(-1, 1.0f);
    a[1] = theirs_mksf(-2, 2.0f);
    b = a[1];
    c = ours_mksf(b.s - 1, b.f + 1.0f);
    return (int)(theirs_sf(a[0], 1) + ours_sf(b, 1) + theirs_sf(c, 1)) ;
}
