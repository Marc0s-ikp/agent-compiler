#include <limits.h>
/* The expected values are the C90 rules (27.5.2), computed from this target's long and int,
   not from GCC's own literal rules for the two decimal literals above INT_MAX; every other
   expected value is the same expression compiled by GCC: GCC's default is C17, where an unsuffixed decimal literal
   above INT_MAX that does not fit long is long long ((-2147483648 < 0) is then 1 on riscv32).
   The compiler follows C90: int, long, unsigned long for decimal; int, unsigned int, long,
   unsigned long for hex; unsigned int, unsigned long with a u suffix. */
#define DEC_FITS_LONG (2147483648ULL <= (unsigned long long)LONG_MAX)  /* is 2147483648 a (signed) long? */

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
    int neg_lt_zero = DEC_FITS_LONG;  /* -2147483648 < 0: true only if the literal is a signed long */
    if (lit_cmp() != neg_lt_zero) return 1;
    if (lit_cmp_const() != neg_lt_zero) return 2;
    if (lit_cmp_switch(neg_lt_zero) != 1) return 3;
    if (lit_hex_cmp() != (0x80000000 > 0) || lit_hex_cmp_const() != (0x80000000 > 0)) return 4;  /* same in GCC */
    if (lit_size_dec() != sizeof(long)) return 5;      /* long, or unsigned long */
    if (lit_size_hex() != sizeof(0x80000000)) return 6;  /* unsigned int in both rule sets */
    if (lit_size_unsigned() != sizeof(2147483648u)) return 7;
    if (lit_size_int() != sizeof(2147483647)) return 8;
    if (lit_size_neg() != sizeof(long)) return 9;
    if (lit_signed_min() != ((-2147483647 - 1) < 0)) return 10;
    return 0;
}
