/* Integer locals only (no control flow): narrow stores, widening, shifts,
   compare, inc/dec, compound assignment, comma (IR step 1). Each failed
   check adds its own bit to 'bad'; the result is 0 when all hold. */
int f()
{
    signed char sc = -3;
    unsigned char uc = 250;
    short sh = -2;
    unsigned short us = 65535;
    int i = 7;
    unsigned u = 4000000000u;
    int bad = 0;

    bad += (sc + 1 != -2) * 1;
    bad += (uc + 10 != 260) * 2;
    bad += (sh * 3 != -6) * 4;
    bad += (us + 1 != 65536) * 8;
    uc += 10;
    bad += (uc != 4) * 16;
    sc--;
    bad += (sc != -4) * 32;
    bad += ((u >> 28) != 14) * 64;
    bad += (((i << 2) != 28) + ((-i >> 1) != -4)) * 128;
    bad += ((u < 5) + !(u > 5)) * 256;
    bad += (((i++, i) != 8) + (i-- != 8) + (--i != 6)) * 512;
    bad += ((~i != -7) + (-i != -6) + !i) * 1024;
    bad += ((i % 4 != 2) + (-i / 4 != -1) + ((u % 7) != 4000000000u % 7)) * 2048;
    return bad;
}
