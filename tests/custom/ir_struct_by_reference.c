/* A struct over 8 bytes is passed by reference to a copy the caller makes. The
   callee overwrites its parameter; the caller's object must be unchanged. Also
   as the middle argument (neighbours keep their registers) and as the 9th
   argument, where the pointer itself goes on the stack.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct Big { int a; int b; int c; };

int ours_clobber(struct Big s)
{ s.a = 100; s.b = 200; s.c = 300; return s.a + s.b + s.c; }

int ours_middle(int x, struct Big s, int y)
{ return x * 1000 + s.a * 100 + s.b * 10 + s.c + y; }

int ours_ninth(int a, int b, int c, int d, int e, int f, int g, int h, struct Big s, int tail)
{ s.a = s.a + 1; return a + h + s.a * 100 + s.b * 10 + s.c + tail * 1000; }

int ours_twice(struct Big s, struct Big t)
{ t.a = s.c; return s.a * 100 + s.b * 10 + s.c + t.a * 1000 + t.b * 10000; }

int theirs_clobber(struct Big s);
int theirs_middle(int x, struct Big s, int y);
int theirs_ninth(int a, int b, int c, int d, int e, int f, int g, int h, struct Big s, int tail);
int theirs_twice(struct Big s, struct Big t);

double call_0(void)
{
    struct Big s; s.a = 1; s.b = 2; s.c = 3;
    return theirs_clobber(s) * 1000 + s.a * 100 + s.b * 10 + s.c;
}

double call_1(void)
{
    struct Big s; s.a = 1; s.b = 2; s.c = 3;
    return theirs_middle(7, s, 9);
}

double call_2(void)
{
    struct Big s; s.a = 1; s.b = 2; s.c = 3;
    return theirs_ninth(1, 2, 3, 4, 5, 6, 7, 8, s, 9) * 10 + s.a;
}

double call_3(void)
{
    struct Big s, t; s.a = 1; s.b = 2; s.c = 3; t.a = 4; t.b = 5; t.c = 6;
    return theirs_twice(s, t) + t.a * 100000;
}
