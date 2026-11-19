/* Every integer operator on 32-bit locals with run-time operands, for the
   AArch64 backend's step A1 (signed and unsigned division, remainder, shifts,
   compares, unary operators, narrow and 64-bit-immediate constants).
   Each failed check adds its own bit to 'bad'. */
int f()
{
    int a = 7;
    int b = -3;
    unsigned u = 4000000000u;
    unsigned v = 7;
    char c = 200;
    signed char sc = -2;
    short s = -300;
    int bad = 0;

    bad += (a * b != -21) * 1;
    bad += (a / b != -2) * 2;
    bad += (a % b != 1) * 4;
    bad += (b / a != 0) * 8;
    bad += (b % a != -3) * 16;
    bad += (u / v != 571428571u) * 32;
    bad += (u % v != 3u) * 64;
    bad += (b >> 1 != -2) * 128;
    bad += ((unsigned)b >> 28 != 15u) * 256;
    bad += (a << 3 != 56) * 512;
    bad += ((a & b) != 5) * 1024;
    bad += ((a | b) != -1) * 2048;
    bad += ((a ^ b) != -6) * 4096;
    bad += (~a != -8) * 8192;
    bad += (-a != -7) * 16384;
    bad += (!a != 0) * 32768;
    bad += (a < b) * 65536;
    bad += (u < v) * 131072;
    bad += (c != 200) * 262144;
    bad += (sc != -2) * 524288;
    bad += (s != -300) * 1048576;
    bad += (c + sc != 198) * 2097152;
    bad += (u + u != 3705032704u) * 4194304;
    bad += ((a >= 7) + (a <= 7) + (a != 7) + (a == 7) != 3) * 8388608;
    return bad;
}
