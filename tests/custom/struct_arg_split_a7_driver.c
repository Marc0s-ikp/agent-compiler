struct P { int a; int b; };

int ours_split(int a, int b, int c, int d, int e, int f, int g, struct P s, int tail);
int call_split(void);

int theirs_split(int a, int b, int c, int d, int e, int f, int g, struct P s, int tail)
{
    return s.a * 100 + s.b * 10 + tail + a + b + c + d + e + f + g;
}

int main()
{
    struct P s;
    s.a = 4; s.b = 5;
    if (ours_split(1, 2, 3, 4, 5, 6, 7, s, 9) != 487) return 1;
    if (call_split() != 487) return 2;
    return 0;
}
