/* &a[i] equals a + i for every i including 0 and one past the end, in a local
   and a global array, one and two dimensions. Each function returns a bit per
   check that held. */
int g1[5];
int g2[3][4];

int local_checks(void) {
    int a[5];
    int b[3][4];
    int i, bits = 0;
    for (i = 0; i <= 5; i++) if (&a[i] == a + i) bits |= 1 << i;
    for (i = 0; i <= 3; i++) if (&b[i] == b + i) bits |= 1 << (i + 8);
    for (i = 0; i <= 4; i++) if (&b[1][i] == b[1] + i) bits |= 1 << (i + 16);
    return bits;
}

int global_checks(void) {
    int i, bits = 0;
    for (i = 0; i <= 5; i++) if (&g1[i] == g1 + i) bits |= 1 << i;
    for (i = 0; i <= 3; i++) if (&g2[i] == g2 + i) bits |= 1 << (i + 8);
    for (i = 0; i <= 4; i++) if (&g2[1][i] == g2[1] + i) bits |= 1 << (i + 16);
    return bits;
}

/* Writes through &g[i] land in g[i]. */
int write_through(void) {
    int i, sum = 0;
    for (i = 0; i < 5; i++) *(&g1[i]) = i * i;
    for (i = 0; i < 5; i++) sum += g1[i];
    return sum;
}
