/* A returned struct is a value: take a member of it, pass it straight on as
   an argument, or hand a parameter back unchanged. */
struct P { int a; int b; };
struct Big { int a; int b; int c; };

struct P mkp(int a, int b) { struct P p; p.a = a; p.b = b; return p; }
struct Big mkbig(int a) { struct Big s; s.a = a; s.b = a + 1; s.c = a + 2; return s; }
int sump(struct P p) { return p.a * 10 + p.b; }
int sumbig(struct Big s) { return s.a * 100 + s.b * 10 + s.c; }
struct P same_p(struct P p) { return p; }
struct Big same_big(struct Big s) { return s; }

int member_of_call(void) { return mkp(3, 4).b; }                 /* 4 */
int member_of_big_call(void) { return mkbig(5).c; }              /* 7 */
int call_as_arg(void) { return sump(mkp(6, 7)); }                /* 67 */
int big_call_as_arg(void) { return sumbig(mkbig(1)); }           /* 123 */
int return_param(void) { return same_p(mkp(8, 9)).a; }           /* 8 */
int return_big_param(void)
{
    struct Big s;
    s = same_big(mkbig(2));
    return s.a * 100 + s.b * 10 + s.c;                           /* 234 */
}
int param_returned_unchanged(void)
{
    struct P p;
    struct P q;
    p.a = 1; p.b = 2;
    q = same_p(p);
    q.a = 50;
    return p.a * 100 + q.a + q.b;                                /* 100+50+2 */
}
