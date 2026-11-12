float i2f(int i);
float u2f_cast(unsigned u);
double u2d_exact(unsigned u);
float d2f(double d);
float assign_i2f(int i);
float init_i2f(int i);
float ret_i2f(int i);
float ret_u2f(unsigned u);
float ret_d2f(double d);
float assign_d2f(double d);

int main(void)
{
    /* 2^24 + 3 is a tie, rounds to the even neighbour 2^24 + 4 */
    if (i2f(16777219) != 16777220.0f) return 1;
    if (i2f(-16777219) != -16777220.0f) return 2;
    /* 2^24 + 1 is a tie, rounds down to the even 2^24 */
    if (i2f(16777217) != 16777216.0f) return 3;
    /* 2^24 + 7 is not a tie: nearest is 2^24 + 8, truncation gives +6 */
    if (i2f(16777223) != 16777224.0f) return 4;
    if (i2f(-16777223) != -16777224.0f) return 5;
    if (u2f_cast(0xFFFFFFFFu) != 4294967296.0f) return 6;
    if (u2f_cast(0x80000081u) != 2147483904.0f) return 7;
    if (u2f_cast(0xFFFFFF80u) != 4294967296.0f) return 8; /* tie, even is 2^32 */
    if (u2d_exact(0xFFFFFFFFu) != 4294967295.0) return 9;
    if (u2d_exact(3000000001u) != 3000000001.0) return 10;
    /* 1 + 3*2^-25 rounds up to 1 + 2^-23 */
    if (d2f(1.0000000894069671630859375) != 1.00000011920928955078125f) return 11;
    if (d2f(-1.0000000894069671630859375) != -1.00000011920928955078125f) return 12;
    /* 1 + 2^-24 is a tie, rounds to even 1.0 */
    if (d2f(1.000000059604644775390625) != 1.0f) return 13;
    if (assign_i2f(16777219) != 16777220.0f) return 14;
    if (assign_i2f(-16777219) != -16777220.0f) return 15;
    if (init_i2f(16777219) != 16777220.0f) return 16;
    if (ret_i2f(16777219) != 16777220.0f) return 17;
    if (ret_i2f(-16777219) != -16777220.0f) return 18;
    if (ret_u2f(0xFFFFFFFFu) != 4294967296.0f) return 19;
    if (ret_d2f(1.0000000894069671630859375) != 1.00000011920928955078125f) return 20;
    if (assign_d2f(1.0000000894069671630859375) != 1.00000011920928955078125f) return 21;
    return 0;
}
