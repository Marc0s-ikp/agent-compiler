struct W1 { int a; };
struct W2 { int a; int b; };
struct B8 { char c[8]; };

struct W1 ours_w1(int x);
struct W2 ours_w2(int x);
struct B8 ours_b8(int x);
int call_w1(void);
int call_w2(void);
int call_b8(void);

struct W1 theirs_w1(int x) { struct W1 s; s.a = x + 1; return s; }
struct W2 theirs_w2(int x) { struct W2 s; s.a = x; s.b = x * 2; return s; }
struct B8 theirs_b8(int x)
{
    struct B8 s;
    int i;
    for (i = 0; i < 8; i++) s.c[i] = x + i;
    return s;
}

int main()
{
    struct W1 a; struct W2 b; struct B8 c;
    int i, sum;
    a = ours_w1(4);
    if (a.a != 5) return 1;
    b = ours_w2(4);
    if (b.a != 4 || b.b != 8) return 2;
    c = ours_b8(20);
    for (i = 0; i < 8; i++) if (c.c[i] != 20 + i) return 3;
    if (call_w1() != 5) return 4;
    if (call_w2() != 408) return 5;
    sum = call_b8();
    if (sum != (((((((0*2+1)*2+2)*2+3)*2+4)*2+5)*2+6)*2+7)) return 6;
    return 0;
}
