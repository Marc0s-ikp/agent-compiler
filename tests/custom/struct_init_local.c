/* Local struct initialisers: unlisted members must be zero, nested braces,
   arrays inside, array of structs, and an initialiser for a static local. */
struct P { int x; int y; };
struct Mix { char c; int i; double d; };
struct Arr { int v[3]; char tag; };
struct Line { struct P a; struct P b; };

int full(void) { struct P p = { 3, 4 }; return p.x * 10 + p.y; }                 /* 34 */

int zero_rest(void)
{
    int junk[8];
    struct Mix m = { 'x' };                  /* i and d must be 0 */
    int i;
    for (i = 0; i < 8; i++) junk[i] = -1;
    return (m.c == 'x') * 100 + (m.i == 0) * 10 + (m.d == 0.0);   /* 111 */
}

int zero_nested(void)
{
    struct Line l = { { 1 } };               /* a.y, b.x, b.y are 0 */
    return l.a.x * 1000 + l.a.y * 100 + l.b.x * 10 + l.b.y;   /* 1000 */
}

int zero_array_member(void)
{
    struct Arr a = { { 9 } };
    return a.v[0] * 100 + a.v[1] * 10 + a.v[2] + a.tag;      /* 900 */
}

int array_of_structs(void)
{
    struct P a[3] = { { 1, 2 }, { 3, 4 } };
    return a[0].x + a[1].y * 10 + a[2].x * 100 + a[2].y * 1000;   /* 41 */
}

int elision(void)
{
    struct Line l = { 1, 2, 3, 4 };
    struct Arr a = { 5, 6, 7, 'h' };
    return l.a.x * 1000 + l.a.y * 100 + l.b.x * 10 + l.b.y + a.v[2] + (a.tag == 'h');   /* 1234+8 */
}

int char_array_member(void)
{
    struct { char name[8]; int n; } r = { "abc", 5 };
    return r.name[0] + r.name[2] + r.name[3] + r.n;       /* 97+99+0+5 */
}

int doubles(void)
{
    struct Mix m = { 1, 2, 3.5 };
    return (int)(m.d * 2.0) + m.c + m.i;                  /* 7+1+2 */
}

int static_local(void)
{
    static struct P p = { 6, 7 };
    p.x++;
    return p.x * 10 + p.y;
}
int static_local_twice(void) { static_local(); return static_local(); }   /* 8*10+7 */

int reinit_each_call_helper(int n)
{
    struct P p = { 5, 6 };                   /* fresh on every call */
    p.x += n;
    return p.x;
}
int reinit(void) { reinit_each_call_helper(10); return reinit_each_call_helper(1); }   /* 6 */
