/* A struct of 5 to 8 bytes travels in two a registers (low word in the first),
   as an argument and as a return value: {int,int}, char[5], {short x3} (6 bytes)
   and {char,int} (8 bytes with padding).
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct P8 { int a; int b; };
struct P5 { char c[5]; };
struct P6 { short a; short b; short c; };
struct P8m { char a; int b; };

int ours_g8(struct P8 s, int n)
{ return s.a * 100 + s.b * 10 + n; }

int ours_g5(struct P5 s, int n)
{ return s.c[0] * 10000 + s.c[1] * 1000 + s.c[2] * 100 + s.c[3] * 10 + s.c[4] + n; }

int ours_g6(struct P6 s, int n)
{ return s.a * 100 + s.b * 10 + s.c + n; }

int ours_g8m(struct P8m s, int n)
{ return s.a * 100 + s.b + n; }

struct P8 ours_n8(int x)
{ struct P8 s; s.a = x; s.b = x * 2; return s; }

struct P5 ours_n5(int x)
{ struct P5 s; int i; for (i = 0; i < 5; i++) s.c[i] = x + i; return s; }

struct P6 ours_n6(int x)
{ struct P6 s; s.a = x; s.b = -x; s.c = x * 3; return s; }

struct P8m ours_n8m(int x)
{ struct P8m s; s.a = x; s.b = x * 1000; return s; }

int theirs_g8(struct P8 s, int n);
int theirs_g5(struct P5 s, int n);
int theirs_g6(struct P6 s, int n);
int theirs_g8m(struct P8m s, int n);
struct P8 theirs_n8(int x);
struct P5 theirs_n5(int x);
struct P6 theirs_n6(int x);
struct P8m theirs_n8m(int x);

double call_0(void)
{
    struct P8 s; s.a = 1; s.b = 2;
    return theirs_g8(s, 3);
}

double call_1(void)
{
    struct P5 s; s.c[0] = 1; s.c[1] = 2; s.c[2] = 3; s.c[3] = 4; s.c[4] = 5;
    return theirs_g5(s, 6);
}

double call_2(void)
{
    struct P6 s; s.a = 1; s.b = -2; s.c = 3;
    return theirs_g6(s, 4);
}

double call_3(void)
{
    struct P8m s; s.a = 5; s.b = 70000;
    return theirs_g8m(s, 1);
}

double call_4(void)
{
    struct P8 v;
    return (v = theirs_n8(7), v.a * 100 + v.b);
}

double call_5(void)
{
    struct P5 v;
    return (v = theirs_n5(10), v.c[0] * 10000 + v.c[2] * 100 + v.c[4]);
}

double call_6(void)
{
    struct P6 v;
    return (v = theirs_n6(4), v.a * 100 + v.b * 10 + v.c);
}

double call_7(void)
{
    struct P8m v;
    return (v = theirs_n8m(9), v.a * 100000 + v.b);
}
