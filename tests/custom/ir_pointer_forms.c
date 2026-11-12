/* Pointer arithmetic forms (D21 21.10): index signedness and width, ++ -- += -=,
   differences for several element sizes, casts to and from integers, null
   pointer forms, unsigned pointer compares, and a (void) cast. */
int idx(int *mid, signed char sc, unsigned char uc, short s, unsigned short us) {
    return mid[sc] + mid[uc] * 2 + mid[s] * 3 + mid[us] * 4;
}

int walk(int *p, int *end) {
    int sum = 0;
    while (p < end) sum += *p++;
    p -= 2;
    sum += *p * 1000;
    p += 1;
    sum += *p * 100000;
    sum += *--p;
    sum += *p--;
    return sum + (int)(p - (end - 5)) * 10000000;
}

int diffs(char *c, short *s, int *i, char *c2, short *s2, int *i2) {
    return (c2 - c) + 10 * (s2 - s) + 100 * (i2 - i);
}

int roundtrip(int *p) {
    unsigned u = (unsigned)p;
    int *q = (int *)u;
    return q == p;
}

int null_forms(int *p) {
    int r = 0;
    if (p == 0) r += 1;
    if (!p) r += 2;
    if (p) r += 4;
    if (p != (int *)0) r += 8;
    r += p ? 16 : 32;
    return r;
}

int high_low(void) {
    int *a = (int *)0x80000000u;
    int *b = (int *)16;
    return (a < b) * 1 + (a > b) * 2 + (a >= b) * 4 + (a <= b) * 8;
}

int negative_address(void) {
    int *p = (int *)-4;
    return (int)p == -4 && p + 1 == (int *)0 && 1 + p == 0;
}

int void_cast(int x) {
    int y = 3;
    (void)(y = x);
    return y;
}
