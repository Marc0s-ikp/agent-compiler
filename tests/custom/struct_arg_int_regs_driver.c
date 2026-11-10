struct W1 { int a; };
struct W2 { int a; int b; };
struct B3 { char a; char b; char c; };

int ours_w1(int x, struct W1 s, int y);
int ours_w2(int x, struct W2 s, int y);
int ours_b3(struct B3 s, int y);
int call_w1(void);
int call_w2(void);
int call_b3(void);

int theirs_w1(int x, struct W1 s, int y) { return x * 100 + s.a * 10 + y; }
int theirs_w2(int x, struct W2 s, int y) { return x * 1000 + s.a * 100 + s.b * 10 + y; }
int theirs_b3(struct B3 s, int y) { return s.a * 1000 + s.b * 100 + s.c * 10 + y; }

int main()
{
    struct W1 w1; struct W2 w2; struct B3 b3;
    w1.a = 2; w2.a = 2; w2.b = 3; b3.a = 1; b3.b = 2; b3.c = 3;
    if (ours_w1(1, w1, 3) != 123) return 1;
    if (ours_w2(1, w2, 4) != 1234) return 2;
    if (ours_b3(b3, 4) != 1234) return 3;
    if (call_w1() != 123) return 4;
    if (call_w2() != 1234) return 5;
    if (call_b3() != 1234) return 6;
    return 0;
}
