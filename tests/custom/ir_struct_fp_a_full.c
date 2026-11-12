/* The a registers run out: the integer field of a float struct has no register.
   With a0-a6 used, {float,int} is fa0 plus a7; with a0-a7 used it goes whole to
   the stack (not fa0), and {int,double} (16 bytes) is passed by reference with
   the pointer on the stack. {float,float} needs no a register and stays in fa0
   and fa1 even when a0-a7 are all used.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct FI { float f; int i; };
struct ID { int i; double d; };
struct FF { float a; float b; };

double ours_a7fi(int a, int b, int c, int d, int e, int f, int g, struct FI s, int tail)
{ return a + g + s.f * 100.0 + s.i * 10.0 + tail; }

double ours_a8fi(int a, int b, int c, int d, int e, int f, int g, int h, struct FI s, int tail)
{ return a + h + s.f * 100.0 + s.i * 10.0 + tail; }

double ours_a7id(int a, int b, int c, int d, int e, int f, int g, struct ID s, int tail)
{ return a + g + s.d * 100.0 + s.i * 10.0 + tail; }

double ours_a8id(int a, int b, int c, int d, int e, int f, int g, int h, struct ID s, int tail)
{ return a + h + s.d * 100.0 + s.i * 10.0 + tail; }

double ours_a8ff(int a, int b, int c, int d, int e, int f, int g, int h, struct FF s, int tail)
{ return a + h + s.a * 100.0 + s.b * 10.0 + tail; }

double theirs_a7fi(int a, int b, int c, int d, int e, int f, int g, struct FI s, int tail);
double theirs_a8fi(int a, int b, int c, int d, int e, int f, int g, int h, struct FI s, int tail);
double theirs_a7id(int a, int b, int c, int d, int e, int f, int g, struct ID s, int tail);
double theirs_a8id(int a, int b, int c, int d, int e, int f, int g, int h, struct ID s, int tail);
double theirs_a8ff(int a, int b, int c, int d, int e, int f, int g, int h, struct FF s, int tail);

double call_0(void)
{
    struct FI s; s.f = 1.5f; s.i = 2;
    return theirs_a7fi(1, 2, 3, 4, 5, 6, 7, s, 9);
}

double call_1(void)
{
    struct FI s; s.f = 1.5f; s.i = 2;
    return theirs_a8fi(1, 2, 3, 4, 5, 6, 7, 8, s, 9);
}

double call_2(void)
{
    struct ID s; s.i = 2; s.d = 1.5;
    return theirs_a7id(1, 2, 3, 4, 5, 6, 7, s, 9);
}

double call_3(void)
{
    struct ID s; s.i = 2; s.d = 1.5;
    return theirs_a8id(1, 2, 3, 4, 5, 6, 7, 8, s, 9);
}

double call_4(void)
{
    struct FF s; s.a = 1.5f; s.b = 2.5f;
    return theirs_a8ff(1, 2, 3, 4, 5, 6, 7, 8, s, 9);
}
