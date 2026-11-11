/* Loops without parameters (IR step 2): do-while, for with missing parts,
   break and continue in nested loops, declarations in loop bodies, empty
   statements. Each failed check adds its own bit to 'bad'. */
int f()
{
    int bad = 0;
    int i, j, n;

    i = 0;
    do { i++; } while (i < 5);
    bad += (i != 5) * 1;

    i = 10;
    do { i++; } while (0);
    bad += (i != 11) * 2;

    n = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (j == 2) continue;
            if (j == 3) break;
            n += 1;
        }
        if (i == 2) continue;
        n += 100;
    }
    bad += (n != 308) * 4;

    n = 0;
    for (;;) {
        n++;
        if (n > 6) break;
    }
    bad += (n != 7) * 8;

    n = 0;
    i = 0;
    for (; i < 3;) { n += i; i++; }
    bad += (n != 3) * 16;

    n = 0;
    for (i = 0; i < 5; i++) {
        int sq = i * i;
        n += sq;
    }
    bad += (n != 30) * 32;

    i = 0;
    while (i < 3) i++;
    ;
    bad += (i != 3) * 64;

    n = 0;
    i = 0;
    do {
        i++;
        if (i == 2) continue;
        n += i;
    } while (i < 4);
    bad += (n != 8) * 128;

    i = 0;
    while (1) {
        do { i++; if (i > 2) break; } while (1);
        if (i > 5) break;
    }
    bad += (i != 6) * 256;
    return bad;
}
