/* All eight a registers are taken: the struct goes on the stack, then the
   next argument follows it (8 bytes later). */
struct P { int a; int b; };

int ours_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct P s, int tail)
{
    return s.a * 1000 + s.b * 100 + tail * 10 + h - a;
}

int theirs_stack(int a, int b, int c, int d, int e, int f, int g, int h, struct P s, int tail);

int call_stack(void)
{
    struct P s;
    s.a = 3; s.b = 4;
    return theirs_stack(1, 2, 3, 4, 5, 6, 7, 8, s, 9);
}
