struct FF { float a; float b; };
struct DD { double a; double b; };
struct FD { float a; double b; };
struct FA2 { float v[2]; };
struct FA1 { float v[1]; };
struct Nest { struct FF in; };
struct Nest2 { struct F1 { float p; } x; struct F1 y; };

float ours_ff(struct FF s);
double ours_dd(struct DD s);
double ours_fd(struct FD s);
float ours_fa2(struct FA2 s);
float ours_fa1(struct FA1 s);
float ours_nest(struct Nest s);
float ours_nest2(struct Nest2 s);
struct FF ours_mkff(float x);
struct DD ours_mkdd(double x);
struct FD ours_mkfd(float x);
struct FA2 ours_mkfa2(float x);
float call_ff(void);
double call_dd(void);
double call_fd(void);
float call_fa2(void);
float call_fa1(void);
float call_nest(void);
float call_nest2(void);
float call_mkff(void);
double call_mkdd(void);
double call_mkfd(void);
float call_mkfa2(void);

float theirs_ff(struct FF s) { return s.a * 10.0f + s.b; }
double theirs_dd(struct DD s) { return s.a * 10.0 + s.b; }
double theirs_fd(struct FD s) { return s.a * 10.0 + s.b; }
float theirs_fa2(struct FA2 s) { return s.v[0] * 10.0f + s.v[1]; }
float theirs_fa1(struct FA1 s) { return s.v[0] * 10.0f; }
float theirs_nest(struct Nest s) { return s.in.a * 10.0f + s.in.b; }
float theirs_nest2(struct Nest2 s) { return s.x.p * 10.0f + s.y.p; }
struct FF theirs_mkff(float x) { struct FF s; s.a = x; s.b = x + 1.0f; return s; }
struct DD theirs_mkdd(double x) { struct DD s; s.a = x; s.b = x + 1.0; return s; }
struct FD theirs_mkfd(float x) { struct FD s; s.a = x; s.b = x + 0.5; return s; }
struct FA2 theirs_mkfa2(float x) { struct FA2 s; s.v[0] = x; s.v[1] = x * 2.0f; return s; }

int main()
{
    struct FF ff; struct DD dd; struct FD fd; struct FA2 fa2; struct FA1 fa1;
    struct Nest n; struct Nest2 n2;
    ff.a = 1.0f; ff.b = 2.0f; dd.a = 1.0; dd.b = 2.0; fd.a = 1.0f; fd.b = 2.0;
    fa2.v[0] = 1.0f; fa2.v[1] = 2.0f; fa1.v[0] = 3.0f;
    n.in = ff; n2.x.p = 1.0f; n2.y.p = 2.0f;
    if (ours_ff(ff) != 12.0f) return 1;
    if (ours_dd(dd) != 12.0) return 2;
    if (ours_fd(fd) != 12.0) return 3;
    if (ours_fa2(fa2) != 12.0f) return 4;
    if (ours_fa1(fa1) != 30.0f) return 5;
    if (ours_nest(n) != 12.0f) return 6;
    if (ours_nest2(n2) != 12.0f) return 7;
    ff = ours_mkff(3.0f);
    if (ff.a != 3.0f || ff.b != 4.0f) return 8;
    dd = ours_mkdd(3.0);
    if (dd.a != 3.0 || dd.b != 4.0) return 9;
    fd = ours_mkfd(3.0f);
    if (fd.a != 3.0f || fd.b != 3.5) return 10;
    fa2 = ours_mkfa2(3.0f);
    if (fa2.v[0] != 3.0f || fa2.v[1] != 6.0f) return 11;
    if (call_ff() != 12.0f) return 12;
    if (call_dd() != 12.0) return 13;
    if (call_fd() != 12.0) return 14;
    if (call_fa2() != 12.0f) return 15;
    if (call_fa1() != 30.0f) return 16;
    if (call_nest() != 12.0f) return 17;
    if (call_nest2() != 12.0f) return 18;
    if (call_mkff() != 34.0f) return 19;
    if (call_mkdd() != 34.0) return 20;
    if (call_mkfd() != 33.5) return 21;
    if (call_mkfa2() != 36.0f) return 22;
    return 0;
}
