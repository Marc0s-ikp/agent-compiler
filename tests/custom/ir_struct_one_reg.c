/* A struct of up to 4 bytes travels in one a register, as an argument and as a
   return value (struct {int}, {short,short}, {char,char,char}). Sub-word
   members must keep their sign: s.b is -3.
   Our functions are ours_*; the GCC-compiled driver defines theirs_* and
   calls both ways, so caller and callee are always checked against GCC. */
struct S1 { int a; };
struct S2 { short a; short b; };
struct S3 { char a; char b; char c; };

int ours_f1(struct S1 s, int n)
{ return s.a * 10 + n; }

int ours_f2(struct S2 s, int n)
{ return s.a * 100 + s.b * 10 + n; }

int ours_f3(struct S3 s, int n)
{ return s.a * 1000 + s.b * 100 + s.c * 10 + n; }

struct S1 ours_m1(int x)
{ struct S1 s; s.a = x * 2; return s; }

struct S2 ours_m2(int x)
{ struct S2 s; s.a = x; s.b = -x; return s; }

struct S3 ours_m3(int x)
{ struct S3 s; s.a = x; s.b = x + 1; s.c = x + 2; return s; }

int theirs_f1(struct S1 s, int n);
int theirs_f2(struct S2 s, int n);
int theirs_f3(struct S3 s, int n);
struct S1 theirs_m1(int x);
struct S2 theirs_m2(int x);
struct S3 theirs_m3(int x);

double call_0(void)
{
    struct S1 s; s.a = 4;
    return theirs_f1(s, 3);
}

double call_1(void)
{
    struct S2 s; s.a = 7; s.b = -3;
    return theirs_f2(s, 1);
}

double call_2(void)
{
    struct S3 s; s.a = 1; s.b = 2; s.c = 3;
    return theirs_f3(s, 4);
}

double call_3(void)
{
    struct S1 v;
    return (v = theirs_m1(21), v.a);
}

double call_4(void)
{
    struct S2 v;
    return (v = theirs_m2(5), v.a * 10 + v.b);
}

double call_5(void)
{
    struct S3 v;
    return (v = theirs_m3(6), v.a * 100 + v.b * 10 + v.c);
}
