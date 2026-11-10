struct Big { int a; int b; int c; };
struct Wide { int a; int b; int c; int d; };

struct Big ours_big(int x, int y);
struct Wide ours_wide(int x);
int call_big(void);
int call_wide(void);

struct Big theirs_big(int x, int y)
{
    struct Big s;
    s.a = x; s.b = y; s.c = x + y;
    return s;
}
struct Wide theirs_wide(int x)
{
    struct Wide s;
    s.a = x; s.b = x + 1; s.c = x + 2; s.d = x + 3;
    return s;
}

int main()
{
    struct Big b; struct Wide w;
    b = ours_big(10, 20);
    if (b.a != 10 || b.b != 20 || b.c != 30) return 1;
    w = ours_wide(5);
    if (w.a != 5 || w.b != 6 || w.c != 7 || w.d != 8) return 2;
    if (call_big() != 102030) return 3;
    if (call_wide() != 5678) return 4;
    return 0;
}
