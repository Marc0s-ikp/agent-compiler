/* && and || with side effects on locals, and as values (IR step 2). */
int f()
{
    int bad = 0;
    int a = 0, b = 0, c = 0, r;

    r = (a++ > 0) && (b++ > 0);
    bad += (r != 0 || a != 1 || b != 0) * 1;

    r = (a++ > 0) && (b++ > 0);
    bad += (r != 0 || a != 2 || b != 1) * 2;

    r = (a > 0) || (c++ > 0);
    bad += (r != 1 || c != 0) * 4;

    r = (a < 0) || (c++ > 0);
    bad += (r != 0 || c != 1) * 8;

    r = (a < 0) || (b > 0);
    bad += (r != 1) * 16;

    r = a && b && c;
    bad += (r != 1) * 32;

    r = (a - 2) || (b - 1) || (c - 1);
    bad += (r != 0) * 64;

    r = 5 && 7;
    bad += (r != 1) * 128;

    r = !(a && 0) + (0 || 3);
    bad += (r != 2) * 256;

    if (a && b || 0) r = 3; else r = 4;
    bad += (r != 3) * 512;

    while (a-- && b) c += 10;
    bad += (c != 21 || a != -1) * 1024;
    return bad;
}
