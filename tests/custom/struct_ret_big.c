/* A struct over 8 bytes is returned through a hidden pointer passed in a0;
   the declared arguments shift up by one register (x in a1, y in a2). */
struct Big { int a; int b; int c; };
struct Wide { int a; int b; int c; int d; };

struct Big ours_big(int x, int y)
{
    struct Big s;
    s.a = x; s.b = y; s.c = x + y;
    return s;
}

struct Wide ours_wide(int x)
{
    struct Wide s;
    s.a = x; s.b = x + 1; s.c = x + 2; s.d = x + 3;
    return s;
}

struct Big theirs_big(int x, int y);
struct Wide theirs_wide(int x);

int call_big(void)
{
    struct Big s;
    s = theirs_big(10, 20);
    return s.a * 10000 + s.b * 100 + s.c;   /* 100000+2000+30 */
}

int call_wide(void)
{
    struct Wide s;
    s = theirs_wide(5);
    return s.a * 1000 + s.b * 100 + s.c * 10 + s.d;
}
