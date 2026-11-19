/* A4 (27.5.5): constant folding of 8-byte unsigned values (size_t, from sizeof arithmetic) must agree with
   run time for comparison, division, remainder and right shift. M = sizeof(int) - 5 is the largest value
   of the type (0xFFFF...FFFF of pointer width). Each expression is folded (an enum value, a case label,
   an array size) and also computed at run time from int parameters, so the compiler cannot fold it. A shift count at run time is itself a size_t
   (sizeof(void *) * 8 - k): the compiler does not yet convert an int count to an 8-byte left operand's type.
   The driver computes every expected value with GCC from the same expressions. */
enum {
    C_div = (sizeof(int) - 5) / 3 % 1000,
    C_mod = (sizeof(int) - 5) % 10,
    C_shr_top = (sizeof(int) - 5) >> (sizeof(void *) * 8 - 1),
    C_shr_mid = ((sizeof(int) - 5) >> 3) % 1000,
    C_gt_zero = (sizeof(int) - 5) > 0,
    C_lt_zero = (sizeof(int) - 5) < 0,
    C_ge_one = (sizeof(int) - 5) >= 1,
    C_le_zero = (sizeof(int) - 5) <= 0,
    C_div_self = (sizeof(int) - 5) / (sizeof(int) - 5),
    C_mod_big = (sizeof(int) - 5) % 1000003,
    C_div_big = (sizeof(int) - 6) / (sizeof(int) - 5)
};

int f_div(void) { return C_div; }
int f_mod(void) { return C_mod; }
int f_shr_top(void) { return C_shr_top; }
int f_shr_mid(void) { return C_shr_mid; }
int f_gt_zero(void) { return C_gt_zero; }
int f_lt_zero(void) { return C_lt_zero; }
int f_ge_one(void) { return C_ge_one; }
int f_le_zero(void) { return C_le_zero; }
int f_div_self(void) { return C_div_self; }
int f_mod_big(void) { return C_mod_big; }
int f_div_big(void) { return C_div_big; }

/* case labels */
int case_div(int x)
{
    switch (x) {
    case (sizeof(int) - 5) / 3 % 1000: return 1;
    case (sizeof(int) - 5) % 10 + 2000: return 2;
    case (sizeof(int) - 5) >> (sizeof(void *) * 8 - 1): return 3;
    default: return 0;
    }
}

/* array sizes */
int arr_div(void) { char a[(sizeof(int) - 5) / 3 % 100 + 1]; return sizeof(a); }
int arr_shr(void) { char a[((sizeof(int) - 5) >> (sizeof(void *) * 8 - 2)) + 1]; return sizeof(a); }

/* the same expressions at run time: five, three, ten, k, two, q are variables */
int r_div(int five, int three) { return (sizeof(int) - five) / three % 1000; }
int r_mod(int five, int ten) { return (sizeof(int) - five) % ten; }
int r_shr_top(int five, int k) { return (sizeof(int) - five) >> (sizeof(void *) * 8 - k); }
int r_shr_mid(int five, int two) { return ((sizeof(int) - five) >> (sizeof(char) + two)) % 1000; }
int r_gt_zero(int five, int zero) { return (sizeof(int) - five) > zero; }
int r_lt_zero(int five, int zero) { return (sizeof(int) - five) < zero; }
int r_ge_one(int five, int one) { return (sizeof(int) - five) >= one; }
int r_le_zero(int five, int zero) { return (sizeof(int) - five) <= zero; }
int r_div_self(int five) { return (sizeof(int) - five) / (sizeof(int) - five); }
int r_mod_big(int five, int q) { return (sizeof(int) - five) % q; }
int r_div_big(int five, int six) { return (sizeof(int) - six) / (sizeof(int) - five); }
