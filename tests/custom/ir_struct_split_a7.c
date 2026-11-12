/* Only a7 is left: the first word of a 5 to 8 byte struct goes in a7 and the
   second word is the first outgoing stack word; the next argument follows it.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct P8 { int a; int b; };
struct C7 { char c[7]; };

int ours_sp8(int a, int b, int c, int d, int e, int f, int g, struct P8 s, int tail)
{ return a + b + c + d + e + f + g + s.a * 100 + s.b * 10 + tail; }

int ours_sp7(int a, int b, int c, int d, int e, int f, int g, struct C7 s, int tail)
{ return g + s.c[0] * 100 + s.c[3] * 10 + s.c[6] + tail; }

int theirs_sp8(int a, int b, int c, int d, int e, int f, int g, struct P8 s, int tail);
int theirs_sp7(int a, int b, int c, int d, int e, int f, int g, struct C7 s, int tail);

double call_0(void)
{
    struct P8 s; s.a = 4; s.b = 5;
    return theirs_sp8(1, 2, 3, 4, 5, 6, 7, s, 9);
}

double call_1(void)
{
    struct C7 s; int i; for (i = 0; i < 7; i++) s.c[i] = i + 1;
    return theirs_sp7(1, 2, 3, 4, 5, 6, 7, s, 9);
}
