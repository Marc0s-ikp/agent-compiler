union UF { float f; int i; };
union Big { double d; char c[12]; };

double ours_mix(int a, union UF u, double d, int b);
int ours_big(union Big u);
double call_mix(void);
int call_big(void);

double theirs_mix(int a, union UF u, double d, int b)
{ return a * 1000.0 + u.i * 100.0 + d * 10.0 + b; }
int theirs_big(union Big u)
{
    u.c[0] = 99;
    return u.c[0] + u.c[11];
}

int main()
{
    union UF u; union Big g; int i;
    u.i = 2;
    for (i = 0; i < 12; i++) g.c[i] = i;
    if (ours_mix(1, u, 3.0, 4) != 1234.0) return 1;
    if (ours_big(g) != 110) return 2;
    if (g.c[0] != 0) return 3;
    if (call_mix() != 1234.0) return 4;
    if (call_big() != 11000) return 5;
    return 0;
}
