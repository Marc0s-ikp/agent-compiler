/* Returns of integer-only structs <= 8 bytes: one word in a0, two words in
   a0 and a1. */
struct W1 { int a; };
struct W2 { int a; int b; };
struct B8 { char c[8]; };

struct W1 ours_w1(int x) { struct W1 s; s.a = x + 1; return s; }
struct W2 ours_w2(int x) { struct W2 s; s.a = x; s.b = x * 2; return s; }
struct B8 ours_b8(int x)
{
    struct B8 s;
    int i;
    for (i = 0; i < 8; i++) s.c[i] = x + i;
    return s;
}

struct W1 theirs_w1(int x);
struct W2 theirs_w2(int x);
struct B8 theirs_b8(int x);

int call_w1(void) { struct W1 s; s = theirs_w1(4); return s.a; }
int call_w2(void) { struct W2 s; s = theirs_w2(4); return s.a * 100 + s.b; }
int call_b8(void)
{
    struct B8 s;
    int i, sum;
    s = theirs_b8(10);
    sum = 0;
    for (i = 0; i < 8; i++) sum = sum * 2 + (s.c[i] - 10);   /* 0,1,..7 */
    return sum;
}
