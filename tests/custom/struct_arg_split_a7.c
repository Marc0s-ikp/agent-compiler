/* A two-word struct when only a7 is left: first word in a7, second word is
   the first stack slot (0(sp)); the next argument follows it at 4(sp). */
struct P { int a; int b; };

int ours_split(int a, int b, int c, int d, int e, int f, int g, struct P s, int tail)
{
    return s.a * 100 + s.b * 10 + tail + a + b + c + d + e + f + g;
}

int theirs_split(int a, int b, int c, int d, int e, int f, int g, struct P s, int tail);

int call_split(void)
{
    struct P s;
    s.a = 4; s.b = 5;
    return theirs_split(1, 2, 3, 4, 5, 6, 7, s, 9);
}
