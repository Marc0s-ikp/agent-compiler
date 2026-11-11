/* ?: as a value: nested, with side effects, with mixed types, in conditions
   (IR step 2). */
int f()
{
    int bad = 0;
    int a = 3, b = 0, c = 0, r;
    unsigned u = 7;
    char ch = 100;

    r = a > 2 ? 10 : 20;
    bad += (r != 10) * 1;

    r = a > 5 ? 10 : 20;
    bad += (r != 20) * 2;

    r = a == 3 ? (b == 0 ? 1 : 2) : (b == 0 ? 3 : 4);
    bad += (r != 1) * 4;

    r = a == 4 ? 1 : a == 3 ? 5 : 6;
    bad += (r != 5) * 8;

    r = a ? (b = 5) : (c = 6);
    bad += (r != 5 || b != 5 || c != 0) * 16;

    r = (a ? ch : 0) + 100;
    bad += (r != 200) * 32;

    r = (a - 3 ? -1 : u) > 0;
    bad += (r != 1) * 64;

    r = 1 + (b > 4 ? a * 2 : a * 3) * 2;
    bad += (r != 13) * 128;

    r = (a ? 0 : 1) ? 9 : 8;
    bad += (r != 8) * 256;

    if (a > 1 ? b : c) r = 1; else r = 2;
    bad += (r != 1) * 512;
    return bad;
}
