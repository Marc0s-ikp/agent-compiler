/* struct { float } travels in fa0 and struct { double } in fa0 (flattened to
   the scalar), passing and returning. */
struct F { float x; };
struct D { double x; };

float ours_f(struct F s, int n) { return s.x * n; }
double ours_d(struct D s, int n) { return s.x * n; }
struct F ours_mkf(float v) { struct F s; s.x = v + 1.0f; return s; }
struct D ours_mkd(double v) { struct D s; s.x = v + 1.0; return s; }

float theirs_f(struct F s, int n);
double theirs_d(struct D s, int n);
struct F theirs_mkf(float v);
struct D theirs_mkd(double v);

float call_f(void) { struct F s; s.x = 2.5f; return theirs_f(s, 3); }
double call_d(void) { struct D s; s.x = 2.5; return theirs_d(s, 3); }
float call_mkf(void) { struct F s; s = theirs_mkf(4.0f); return s.x; }
double call_mkd(void) { struct D s; s = theirs_mkd(4.0); return s.x; }
