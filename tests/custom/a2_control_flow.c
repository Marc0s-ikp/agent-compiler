/* Control flow for the AArch64 backend's step A2, with no parameters: every
   compare in both signedness and every direction (including <= and >=),
   negative operands, switch with a negative and a large case, and nested
   &&, || and ?: with side effects (n counts the operands evaluated). Each failed check adds its own bit to 'bad'. */
int f()
{
    unsigned ua = 3000000000u, ub = 5, uc = 5;
    int a = -4, b = 9, c = -4;
    int bad = 0;
    int i, sum = 0, big = 2000000000, neg = -5, n = 0, k;

    bad += !(ub < ua) * 1;
    bad += (ua < ub) * 2;
    bad += !(ub <= ua) * 4;
    bad += !(ub <= uc) * 8;
    bad += (ua <= ub) * 16;
    bad += !(ua > ub) * 32;
    bad += (ub > ua) * 64;
    bad += !(ua >= ub) * 128;
    bad += !(uc >= ub) * 256;
    bad += (ub >= ua) * 512;
    bad += !(a < b) * 1024;
    bad += !(a <= c) * 2048;
    bad += (a > b) * 4096;
    bad += !(b >= a) * 8192;
    bad += (a >= b) * 16384;
    bad += (a != c) * 32768;
    bad += !(a == c) * 65536;

    for (i = -3; i < 3; i = i + 1) sum = sum + i;
    bad += (sum != -3) * 131072;

    for (k = 0; k < 5; k = k + 1) {
        int x = k == 0 ? neg : k == 1 ? 0 : k == 2 ? big : k == 3 ? -big : 7;
        int r;
        switch (x) {
        case -5: r = 1; break;
        case 0: r = 2; break;
        case 2000000000: r = 3; break;
        case -2000000000: r = 4; break;
        default: r = 5;
        }
        if (r != k + 1) bad += 262144;
    }

    n = 0;
    i = ((n = n + 1, 0) && (n = n + 1, 1)) || ((n = n + 1, 1) && (n = n + 1, 0)) || (n = n + 1, 1) ? (n = n + 1, 10) : (n = n + 1, 20);
    bad += (i != 10 || n != 5) * 524288;
    n = 0;
    i = ((n = n + 1, 1) || (n = n + 1, 1)) && (n = n + 1, 0) ? (n = n + 1, 1) : ((n = n + 1, 0) ? 7 : (n = n + 1, 30));
    bad += (i != 30 || n != 4) * 1048576;
    return bad;
}
