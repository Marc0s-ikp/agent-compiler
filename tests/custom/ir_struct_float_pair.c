/* {float,float} travels in fa0 and fa1 (one register per field), also as a
   return value; an int argument before and after it keeps its a registers.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct FF { float a; float b; };

float ours_fa(struct FF s)
{ return s.a * 10.0f + s.b; }

float ours_fb(int n, struct FF s, int m)
{ return n * 100.0f + s.a * 10.0f + s.b + m * 1000.0f; }

struct FF ours_mk(float x)
{ struct FF s; s.a = x; s.b = x + 1.0f; return s; }

float theirs_fa(struct FF s);
float theirs_fb(int n, struct FF s, int m);
struct FF theirs_mk(float x);

double call_0(void)
{
    struct FF s; s.a = 1.5f; s.b = 2.5f;
    return theirs_fa(s);
}

double call_1(void)
{
    struct FF s; s.a = 1.5f; s.b = 2.5f;
    return theirs_fb(3, s, 4);
}

double call_2(void)
{
    struct FF v;
    return (v = theirs_mk(2.5f), v.a * 10.0f + v.b);
}
