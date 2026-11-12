/* Element addresses of global and local arrays, 1-D and 2-D (rows of 3 ints,
   so 12 bytes, not a power of two). The global addresses are returned to the
   driver, which is compiled by GCC and compares them with its own &g1[i],
   g2[i] + j and so on, so a scaling mistake that &a[i] and a + i share cannot
   hide. The local checks compare against byte arithmetic done in this file. */
int g1[5];
int g2[4][3];

void *p1(int i) { return &g1[i]; }
void *q1(int i) { return g1 + i; }
void *p2(int i, int j) { return &g2[i][j]; }
void *q2(int i, int j) { return g2[i] + j; }
void *r2(int i) { return &g2[i]; }
void *s2(int i) { return g2 + i; }

/* One bit per check that held; i = 5 and i = 4 are one past the end. */
int local_checks(void) {
    int a[5];
    int b[4][3];
    int i, j, bits = 0;
    for (i = 0; i <= 5; i++) {
        if (&a[i] == a + i) bits |= 1 << i;
        if ((char *)&a[i] - (char *)a == i * (int)sizeof(int)) bits |= 1 << (i + 6);
    }
    for (i = 0; i <= 4; i++) {
        if (&b[i] == b + i) bits |= 1 << (i + 12);
        if ((char *)&b[i] - (char *)b == i * 3 * (int)sizeof(int)) bits |= 1 << (i + 17);
    }
    for (j = 0; j <= 3; j++) {
        if ((char *)&b[2][j] - (char *)b == (2 * 3 + j) * (int)sizeof(int)) bits |= 1 << (j + 22);
    }
    return bits;
}

/* Writes through &g1[i] land in g1[i]. */
int write_through(void) {
    int i, sum = 0;
    for (i = 0; i < 5; i++) *(&g1[i]) = i * i;
    for (i = 0; i < 5; i++) sum += g1[i];
    return sum;
}
