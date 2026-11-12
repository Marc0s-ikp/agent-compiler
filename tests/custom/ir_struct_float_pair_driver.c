struct FF { float a; float b; };

float ours_fa(struct FF s);
float ours_fb(int n, struct FF s, int m);
struct FF ours_mk(float x);
double call_0(void);
double call_1(void);
double call_2(void);

float theirs_fa(struct FF s)
{ return s.a * 10.0f + s.b; }

float theirs_fb(int n, struct FF s, int m)
{ return n * 100.0f + s.a * 10.0f + s.b + m * 1000.0f; }

struct FF theirs_mk(float x)
{ struct FF s; s.a = x; s.b = x + 1.0f; return s; }

int main(void)
{
    { struct FF s; s.a = 1.5f; s.b = 2.5f; if ((ours_fa(s)) != (17.5)) return 1; }   /* GCC caller, our callee */
    if (call_0() != (17.5)) return 2;   /* our caller, GCC callee */
    { struct FF s; s.a = 1.5f; s.b = 2.5f; if ((ours_fb(3, s, 4)) != (4317.5)) return 3; }   /* GCC caller, our callee */
    if (call_1() != (4317.5)) return 4;   /* our caller, GCC callee */
    { struct FF v; if (((v = ours_mk(2.5f), v.a * 10.0f + v.b)) != (28.5)) return 5; }   /* GCC caller, our callee */
    if (call_2() != (28.5)) return 6;   /* our caller, GCC callee */
    return 0;
}
