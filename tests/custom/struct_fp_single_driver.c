struct F { float x; };
struct D { double x; };

float ours_f(struct F s, int n);
double ours_d(struct D s, int n);
struct F ours_mkf(float v);
struct D ours_mkd(double v);
float call_f(void);
double call_d(void);
float call_mkf(void);
double call_mkd(void);

float theirs_f(struct F s, int n) { return s.x * n; }
double theirs_d(struct D s, int n) { return s.x * n; }
struct F theirs_mkf(float v) { struct F s; s.x = v + 1.0f; return s; }
struct D theirs_mkd(double v) { struct D s; s.x = v + 1.0; return s; }

int main()
{
    struct F f; struct D d;
    f.x = 2.5f; d.x = 2.5;
    if (ours_f(f, 3) != 7.5f) return 1;
    if (ours_d(d, 3) != 7.5) return 2;
    f = ours_mkf(4.0f);
    if (f.x != 5.0f) return 3;
    d = ours_mkd(4.0);
    if (d.x != 5.0) return 4;
    if (call_f() != 7.5f) return 5;
    if (call_d() != 7.5) return 6;
    if (call_mkf() != 5.0f) return 7;
    if (call_mkd() != 5.0) return 8;
    return 0;
}
