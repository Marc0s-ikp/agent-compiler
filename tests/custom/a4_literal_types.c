/* A4 (27.5.2): the C90 type of an integer literal follows the target's sizes. */
enum { Const_cmp = -2147483648 < 0, Const_hex = 0x80000000 > 0 };

int lit_cmp(void) { return -2147483648 < 0; }
int lit_cmp_const(void) { return Const_cmp; }
int lit_cmp_switch(int x)
{
    switch (x) {
    case -2147483648 < 0: return 1;
    default: return 0;
    }
}
int lit_hex_cmp(void) { return 0x80000000 > 0; }
int lit_hex_cmp_const(void) { return Const_hex; }
int lit_size_dec(void) { return sizeof(2147483648); }
int lit_size_hex(void) { return sizeof(0x80000000); }
int lit_size_unsigned(void) { return sizeof(2147483648u); }
int lit_size_int(void) { return sizeof(2147483647); }
int lit_size_neg(void) { return sizeof(-2147483648); }
int lit_signed_min(void) { return (-2147483647 - 1) < 0; }
