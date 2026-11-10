/* struct { double } when fa registers are gone and a0-a6 are used: the low
   word goes in a7 and the high word is the first stack slot. GCC's own callee
   misreads this split under spike pk, so caller and callee are both ours. */
struct D1 { double a; };

double split(double d1, double d2, double d3, double d4, double d5, double d6,
             double d7, double d8, int a, int b, int c, int d, int e, int f, int g,
             struct D1 s, int tail)
{
    struct D1 copy;
    copy = s;
    return copy.a * 10.0 + tail + a + g;
}

double call_split(void)
{
    struct D1 s;
    s.a = 2.5;
    return split(1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, s, 100);
}
