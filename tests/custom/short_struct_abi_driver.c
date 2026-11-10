struct SF { short s; float f; };
struct FS { float f; short s; };
struct SD { short s; double d; };
struct SS { short a; short b; };
struct CS { signed char c; short s; };
struct SSS { short a; short b; short c; };
struct SI { short s; int i; };
struct USF { unsigned short u; float f; };
float theirs_sf(struct SF x, int n) { return x.s * n + x.f; }
struct SF theirs_mksf(short s, float f) { struct SF x; x.s = s; x.f = f; return x; }
float theirs_fs(struct FS x) { return x.f * 2 + x.s; }
struct FS theirs_mkfs(float f, short s) { struct FS x; x.f = f; x.s = s; return x; }
double theirs_sd(struct SD x) { return x.d + x.s; }
struct SD theirs_mksd(short s, double d) { struct SD x; x.s = s; x.d = d; return x; }
int theirs_ss(struct SS x) { return x.a * 1000 + x.b; }
struct SS theirs_mkss(short a, short b) { struct SS x; x.a = a; x.b = b; return x; }
int theirs_cs(struct CS x) { return x.c * 1000 + x.s; }
struct CS theirs_mkcs(signed char c, short s) { struct CS x; x.c = c; x.s = s; return x; }
int theirs_sss(struct SSS x) { return x.a * 10000 + x.b * 100 + x.c; }
struct SSS theirs_mksss(short a, short b, short c) { struct SSS x; x.a = a; x.b = b; x.c = c; return x; }
int theirs_si(struct SI x) { return x.s * 1000 + x.i; }
struct SI theirs_mksi(short s, int i) { struct SI x; x.s = s; x.i = i; return x; }
float theirs_usf(struct USF x) { return x.u + x.f; }
struct USF theirs_mkusf(unsigned short u, float f) { struct USF x; x.u = u; x.f = f; return x; }
/* the integer registers run out: both structs go on the stack */
float theirs_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct SS x, struct SF y)
{ return a + h + x.a + x.b + y.s + y.f; }
float theirs_split(int a, int b, int c, int d, int e, int f, int g, struct SS x, struct CS y)
{ return a + g + x.a + x.b + y.c + y.s; }
extern float ours_sf(struct SF x, int n);
extern struct SF ours_mksf(short s, float f);
extern float ours_fs(struct FS x);
extern struct FS ours_mkfs(float f, short s);
extern double ours_sd(struct SD x);
extern struct SD ours_mksd(short s, double d);
extern int ours_ss(struct SS x);
extern struct SS ours_mkss(short a, short b);
extern int ours_cs(struct CS x);
extern struct CS ours_mkcs(signed char c, short s);
extern int ours_sss(struct SSS x);
extern struct SSS ours_mksss(short a, short b, short c);
extern int ours_si(struct SI x);
extern struct SI ours_mksi(short s, int i);
extern float ours_usf(struct USF x);
extern struct USF ours_mkusf(unsigned short u, float f);
extern float ours_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct SS x, struct SF y);
extern float ours_split(int a, int b, int c, int d, int e, int f, int g, struct SS x, struct CS y);
extern int call_sf(void), call_fs(void), call_sd(void), call_ss(void), call_cs(void);
extern int call_sss(void), call_si(void), call_usf(void), call_stack(void), call_split(void);
extern int copy_chain(void);

int main(void)
{
    struct SF sf; struct FS fs; struct SD sd; struct SS ss; struct CS cs;
    struct SSS sss; struct SI si; struct USF usf;
    sf = ours_mksf(-4, 1.5f);
    if (sf.s != -4 || sf.f != 1.5f) return 1;
    if (ours_sf(sf, 3) != -10.5f) return 2;
    fs = ours_mkfs(0.5f, -6);
    if (fs.f != 0.5f || fs.s != -6) return 3;
    if (ours_fs(fs) != -5.0f) return 4;
    sd = ours_mksd(-100, 0.25);
    if (sd.s != -100 || sd.d != 0.25) return 5;
    if (ours_sd(sd) != -99.75) return 6;
    ss = ours_mkss(-3, -4);
    if (ss.a != -3 || ss.b != -4) return 7;
    if (ours_ss(ss) != -3004) return 8;
    cs = ours_mkcs(-5, -300);
    if (cs.c != -5 || cs.s != -300) return 9;
    if (ours_cs(cs) != -5300) return 10;
    sss = ours_mksss(-1, -2, -3);
    if (sss.a != -1 || sss.b != -2 || sss.c != -3) return 11;
    if (ours_sss(sss) != -10000 - 200 - 3) return 12;
    si = ours_mksi(-7, -8);
    if (si.s != -7 || si.i != -8) return 13;
    if (ours_si(si) != -7008) return 14;
    usf = ours_mkusf(65535, 0.5f);
    if (usf.u != 65535 || usf.f != 0.5f) return 15;
    if (ours_usf(usf) != 65535.5f) return 16;
    ss.a = -10; ss.b = -20; sf.s = -30; sf.f = 4.0f;
    if (ours_stack(1, 2, 3, 4, 5, 6, 7, 8, ss, sf) != 9 - 10 - 20 - 30 + 4) return 17;
    cs.c = -3; cs.s = -400;
    if (ours_split(1, 2, 3, 4, 5, 6, 7, ss, cs) != 8 - 10 - 20 - 3 - 400) return 18;
    /* the same values through GCC's own versions agree */
    if (ours_sf(theirs_mksf(-9, 2.0f), 2) != theirs_sf(ours_mksf(-9, 2.0f), 2)) return 19;

    if (call_sf() != -105) return 21;
    if (call_fs() != -50) return 22;
    if (call_sd() != -9975) return 23;
    if (call_ss() != -3004) return 24;
    if (call_cs() != -5300) return 25;
    if (call_sss() != -10203) return 26;
    if (call_si() != -7008) return 27;
    if (call_usf() != 131071) return 28;
    if (call_stack() != 9 - 10 - 20 - 30 + 4) return 29;
    if (call_split() != 8 - 10 - 20 - 3 - 400) return 30;
    /* a: -1+1 contributes 0; b: -2+2 = 0; c: s=-3, f=3 -> 0 */
    if (copy_chain() != 0) return 31;
    return 0;
}
