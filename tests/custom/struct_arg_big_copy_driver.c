struct Big { int a; int b; int c; };

int ours_clobber(struct Big s);
int ours_middle(int x, struct Big s, int y);
int call_clobber(void);
int call_middle(void);

int theirs_clobber(struct Big s)
{
    s.a = 100; s.b = 200; s.c = 300;
    return s.a + s.b + s.c;
}
int theirs_middle(int x, struct Big s, int y) { return x * 1000 + s.a + s.b + s.c + y; }

int main()
{
    struct Big s;
    s.a = 1; s.b = 2; s.c = 3;
    if (ours_clobber(s) != 600) return 1;
    if (s.a != 1 || s.b != 2 || s.c != 3) return 2;
    if (ours_middle(7, s, 9) != 7015) return 3;
    if (call_clobber() != 600123) return 4;
    if (call_middle() != 7015) return 5;
    return 0;
}
