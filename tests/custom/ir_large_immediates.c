/* Integer constants that do not fit a 12-bit immediate, used with every kind
   of operator (IR step 2). Each failed check adds its own bit to 'bad'. */
int f()
{
    int x = 5;
    unsigned u = 3000000000u;
    int bad = 0;

    bad += (x + 100000 != 100005) * 1;
    bad += ((x & 0x12345) != 5) * 2;
    bad += ((x | 0x7ff800) != 0x7ff805) * 4;
    bad += ((x ^ 0x7fffffff) != 0x7ffffffa) * 8;
    bad += (x * 70000 != 350000) * 16;
    bad += (x - 2048 != -2043) * 32;
    bad += (x < 100000) == 0;
    bad += (x > 100000) * 64;
    bad += (x == 5000000) * 128;
    bad += (x != 5) * 256;
    bad += (-x <= -100000) * 512;
    bad += (u < 2999999999u) * 1024;
    bad += (u / 100000u != 30000u) * 2048;
    bad += ((x << 2) - 4096 != -4076) * 4096;
    return bad;
}
