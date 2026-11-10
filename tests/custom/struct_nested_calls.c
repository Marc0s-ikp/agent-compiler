/* A struct temporary must survive while later arguments are evaluated and
   while other calls run; also recursion with structs by value. */
struct P { int a; int b; };
struct Big { int a; int b; int c; };

struct P mk(int a) { struct P p; p.a = a; p.b = a * 2; return p; }
struct Big mkb(int a) { struct Big s; s.a = a; s.b = a * 2; s.c = a * 3; return s; }
int add(struct P x, struct P y) { return x.a + x.b * 10 + y.a * 100 + y.b * 1000; }
int addb(struct Big x, struct Big y) { return x.a + x.b + x.c + y.a * 100 + y.b * 100 + y.c * 100; }

int two_temps(void) { return add(mk(1), mk(2)); }        /* 1+20+200+4000 */
int two_big_temps(void) { return addb(mkb(1), mkb(2)); } /* 6 + 1200 */
int nested(void) { return add(mk(add(mk(1), mk(0)) - 20), mk(1)); }  /* inner 21, minus 20 is 1; add({1,2},{1,2}) */

/* temporary kept across an unrelated call */
int across_call(void)
{
    struct P p;
    int x;
    p = mk(5);
    x = add(mk(1), mk(1));       /* clobbers all registers */
    return p.a * 10 + p.b + x * 0;
}

/* recursion: pass the struct by value and return the updated one */
struct P step(struct P p, int n)
{
    if (n == 0) return p;
    p.a = p.a + 1;
    p.b = p.b * 2;
    return step(p, n - 1);
}
int recurse(void)
{
    struct P p;
    p.a = 0; p.b = 1;
    p = step(p, 5);
    return p.a * 100 + p.b;       /* 5, 32 */
}

struct Big sumb(struct Big s, int n)
{
    struct Big r;
    if (n == 0) return s;
    r = sumb(s, n - 1);
    r.a = r.a + n;
    r.b = r.b + n * 2;
    r.c = r.c + n * 3;
    return r;
}
int recurse_big(void)
{
    struct Big s;
    s.a = 0; s.b = 0; s.c = 0;
    s = sumb(s, 4);               /* 10, 20, 30 */
    return s.a * 10000 + s.b * 100 + s.c;
}
