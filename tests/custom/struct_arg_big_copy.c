/* A struct over 8 bytes is passed as a pointer to a copy the caller makes.
   The callee may write to its parameter; the caller's object must not change. */
struct Big { int a; int b; int c; };

int ours_clobber(struct Big s)
{
    s.a = 100; s.b = 200; s.c = 300;
    return s.a + s.b + s.c;
}

/* big struct in the middle: the ints around it keep their registers */
int ours_middle(int x, struct Big s, int y) { return x * 1000 + s.a + s.b + s.c + y; }

int theirs_clobber(struct Big s);
int theirs_middle(int x, struct Big s, int y);

int call_clobber(void)
{
    struct Big s;
    int r;
    s.a = 1; s.b = 2; s.c = 3;
    r = theirs_clobber(s);
    return r * 1000 + s.a * 100 + s.b * 10 + s.c;   /* 600 then 123 */
}

int call_middle(void)
{
    struct Big s;
    s.a = 1; s.b = 2; s.c = 3;
    return theirs_middle(7, s, 9);
}
