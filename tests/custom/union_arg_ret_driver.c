union UF { float f; int i; };
union UD { double d; int i; };
union UC { char c; int i; };

int ours_uf(union UF u, int n);
double ours_ud(union UD u, int n);
int ours_uc(union UC u, int n);
union UF ours_mkuf(float v);
union UD ours_mkud(double v);
union UC ours_mkuc(int v);
int call_uf(void);
double call_ud(void);
int call_uc(void);
float call_mkuf(void);
double call_mkud(void);
int call_mkuc(void);

int theirs_uf(union UF u, int n) { return u.i + n; }
double theirs_ud(union UD u, int n) { return u.d + n; }
int theirs_uc(union UC u, int n) { return u.c + n; }
union UF theirs_mkuf(float v) { union UF u; u.f = v; return u; }
union UD theirs_mkud(double v) { union UD u; u.d = v; return u; }
union UC theirs_mkuc(int v) { union UC u; u.i = v; return u; }

int main()
{
    union UF a; union UD b; union UC c;
    a.i = 40; b.d = 40.5; c.i = 40;
    if (ours_uf(a, 2) != 42) return 1;
    if (ours_ud(b, 2) != 42.5) return 2;
    if (ours_uc(c, 2) != 42) return 3;
    a = ours_mkuf(1.5f);
    if (a.f != 1.5f) return 4;
    b = ours_mkud(1.5);
    if (b.d != 1.5) return 5;
    c = ours_mkuc(77);
    if (c.i != 77) return 6;
    if (call_uf() != 42) return 7;
    if (call_ud() != 42.5) return 8;
    if (call_uc() != 42) return 9;
    if (call_mkuf() != 1.5f) return 10;
    if (call_mkud() != 1.5) return 11;
    if (call_mkuc() != 77) return 12;
    return 0;
}
