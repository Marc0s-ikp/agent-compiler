/* Structs with two floating members (also via nesting or a float array) are
   flattened: first member in fa0, second in fa1, whatever their sizes.
   Return uses fa0 and fa1 the same way. */
struct FF { float a; float b; };
struct DD { double a; double b; };
struct FD { float a; double b; };
struct FA2 { float v[2]; };
struct FA1 { float v[1]; };
struct Nest { struct FF in; };
struct Nest2 { struct F1 { float p; } x; struct F1 y; };

float ours_ff(struct FF s) { return s.a * 10.0f + s.b; }
double ours_dd(struct DD s) { return s.a * 10.0 + s.b; }
double ours_fd(struct FD s) { return s.a * 10.0 + s.b; }
float ours_fa2(struct FA2 s) { return s.v[0] * 10.0f + s.v[1]; }
float ours_fa1(struct FA1 s) { return s.v[0] * 10.0f; }
float ours_nest(struct Nest s) { return s.in.a * 10.0f + s.in.b; }
float ours_nest2(struct Nest2 s) { return s.x.p * 10.0f + s.y.p; }

struct FF ours_mkff(float x) { struct FF s; s.a = x; s.b = x + 1.0f; return s; }
struct DD ours_mkdd(double x) { struct DD s; s.a = x; s.b = x + 1.0; return s; }
struct FD ours_mkfd(float x) { struct FD s; s.a = x; s.b = x + 0.5; return s; }
struct FA2 ours_mkfa2(float x) { struct FA2 s; s.v[0] = x; s.v[1] = x * 2.0f; return s; }

float theirs_ff(struct FF s);
double theirs_dd(struct DD s);
double theirs_fd(struct FD s);
float theirs_fa2(struct FA2 s);
float theirs_fa1(struct FA1 s);
float theirs_nest(struct Nest s);
float theirs_nest2(struct Nest2 s);
struct FF theirs_mkff(float x);
struct DD theirs_mkdd(double x);
struct FD theirs_mkfd(float x);
struct FA2 theirs_mkfa2(float x);

float call_ff(void) { struct FF s; s.a = 1.0f; s.b = 2.0f; return theirs_ff(s); }
double call_dd(void) { struct DD s; s.a = 1.0; s.b = 2.0; return theirs_dd(s); }
double call_fd(void) { struct FD s; s.a = 1.0f; s.b = 2.0; return theirs_fd(s); }
float call_fa2(void) { struct FA2 s; s.v[0] = 1.0f; s.v[1] = 2.0f; return theirs_fa2(s); }
float call_fa1(void) { struct FA1 s; s.v[0] = 3.0f; return theirs_fa1(s); }
float call_nest(void) { struct Nest s; s.in.a = 1.0f; s.in.b = 2.0f; return theirs_nest(s); }
float call_nest2(void) { struct Nest2 s; s.x.p = 1.0f; s.y.p = 2.0f; return theirs_nest2(s); }
float call_mkff(void) { struct FF s; s = theirs_mkff(3.0f); return s.a * 10.0f + s.b; }
double call_mkdd(void) { struct DD s; s = theirs_mkdd(3.0); return s.a * 10.0 + s.b; }
double call_mkfd(void) { struct FD s; s = theirs_mkfd(3.0f); return s.a * 10.0 + s.b; }
float call_mkfa2(void) { struct FA2 s; s = theirs_mkfa2(3.0f); return s.v[0] * 10.0f + s.v[1]; }
