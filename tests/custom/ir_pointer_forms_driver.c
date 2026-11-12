int idx(int *mid, signed char sc, unsigned char uc, short s, unsigned short us);
int walk(int *p, int *end);
int diffs(char *c, short *s, int *i, char *c2, short *s2, int *i2);
int roundtrip(int *p);
int null_forms(int *p);
int high_low(void);
int negative_address(void);
int void_cast(int x);

int main()
{
    int x[400];
    int a[5];
    char cb[16];
    short sb[16];
    int ib[16];
    int k;
    for (k = 0; k < 400; k++) x[k] = k;
    for (k = 0; k < 5; k++) a[k] = k + 1;
    if (idx(&x[100], -3, 250, -5, 299) != 2678) return 1;
    if (walk(a, a + 5) != 20504023) return 2;
    if (diffs(cb, sb + 5, ib + 2, cb + 7, sb + 2, ib + 7) != 477) return 3;
    if (!roundtrip(ib) || !roundtrip(0)) return 4;
    if (null_forms(0) != 35 || null_forms(ib) != 28) return 5;
    if (high_low() != 6) return 6;
    if (!negative_address()) return 7;
    if (void_cast(9) != 9) return 8;
    return 0;
}
