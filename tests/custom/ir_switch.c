/* switch without parameters (IR step 2): fallthrough, default in the middle,
   nested switches, break inside a switch inside a loop, continue through a
   switch, and no matching case. */
int f()
{
    int bad = 0;
    int i, n, r;

    n = 0;
    for (i = 0; i < 6; i++) {
        switch (i) {
        case 0: n += 1;
        case 1: n += 10; break;
        default: n += 100;
        case 4: n += 1000; break;
        case 5: continue;
        }
        n += 10000;
    }
    bad += (n != 53221) * 1;

    r = 0;
    switch (3) { case 1: r = 1; break; case 2: r = 2; break; }
    bad += (r != 0) * 2;

    r = 0;
    switch (2) { case 1: r = 1; break; case 2: r = 2; break; }
    bad += (r != 2) * 4;

    r = 0;
    switch (7) { default: r = 5; }
    bad += (r != 5) * 8;

    r = 0;
    i = 1;
    switch (i) {
    case 1:
        switch (i + 1) {
        case 1: r += 1; break;
        case 2: r += 20;
        case 3: r += 300; break;
        }
        r += 4000;
        break;
    case 2: r = -1;
    }
    bad += (r != 4320) * 16;

    r = 0;
    switch (-1) { case -1: r = 6; break; case 1: r = 7; }
    bad += (r != 6) * 32;

    r = 0;
    switch (i = 5) { }
    bad += (i != 5) * 64;

    n = 0;
    i = 0;
    while (i < 5) {
        i++;
        switch (i) {
        case 2: continue;
        case 4: break;
        default: n += i;
        }
        n += 100;
    }
    bad += (n != 1 + 100 + 3 + 100 + 100 + 5 + 100) * 128;

    r = 0;
    {
        char c = 'b';
        switch (c) { case 'a': r = 1; break; case 'b': r = 2; break; }
    }
    bad += (r != 2) * 256;
    return bad;
}
