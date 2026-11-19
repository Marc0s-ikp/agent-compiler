/* Every expected value is the same expression compiled by GCC. The driver is built with -std=gnu90
   (a4_literal_types.gccflags), so GCC types the literals by the C90 rules the compiler follows:
   int, long, unsigned long for decimal; int, unsigned int, long, unsigned long for hex;
   unsigned int, unsigned long with a u suffix. (GCC's default, C17, would make a decimal
   literal above INT_MAX that does not fit long a long long.) */
int lit_cmp(void);
int lit_cmp_const(void);
int lit_cmp_switch(int x);
int lit_hex_cmp(void);
int lit_hex_cmp_const(void);
int lit_size_dec(void);
int lit_size_hex(void);
int lit_size_unsigned(void);
int lit_size_int(void);
int lit_size_neg(void);
int lit_signed_min(void);

int main(void)
{
    if (lit_cmp() != (-2147483648 < 0)) return 1;
    if (lit_cmp_const() != (-2147483648 < 0)) return 2;
    if (lit_cmp_switch(-2147483648 < 0) != 1) return 3;
    if (lit_hex_cmp() != (0x80000000 > 0) || lit_hex_cmp_const() != (0x80000000 > 0)) return 4;
    if (lit_size_dec() != sizeof(2147483648)) return 5;
    if (lit_size_hex() != sizeof(0x80000000)) return 6;
    if (lit_size_unsigned() != sizeof(2147483648u)) return 7;
    if (lit_size_int() != sizeof(2147483647)) return 8;
    if (lit_size_neg() != sizeof(-2147483648)) return 9;
    if (lit_signed_min() != ((-2147483647 - 1) < 0)) return 10;
    return 0;
}
