extern int g1[5];
extern int g2[4][3];
void *p1(int i);
void *q1(int i);
void *p2(int i, int j);
void *q2(int i, int j);
void *r2(int i);
void *s2(int i);
int local_checks(void);
int write_through(void);

int main()
{
    int i, j;
    /* local_checks: a[0..5] twice, b rows 0..4 twice, three more for b[2][j] */
    int want = 0x3F | (0x3F << 6) | (0x1F << 12) | (0x1F << 17) | (0xF << 22);
    for (i = 0; i <= 5; i++) {
        if (p1(i) != (void *)&g1[i]) return 1;
        if (q1(i) != (void *)(g1 + i)) return 2;
    }
    for (i = 0; i <= 4; i++) {
        if (r2(i) != (void *)&g2[i]) return 3;
        if (s2(i) != (void *)(g2 + i)) return 4;
        for (j = 0; j <= 3; j++) {
            if (p2(i, j) != (void *)&g2[i][j]) return 5;
            if (q2(i, j) != (void *)(g2[i] + j)) return 6;
        }
    }
    if ((char *)p2(3, 2) - (char *)g2 != (3 * 3 + 2) * 4) return 7;
    if (local_checks() != want) return 8;
    if (write_through() != 0 + 1 + 4 + 9 + 16) return 9;
    if (g1[3] != 9) return 10;
    return 0;
}
