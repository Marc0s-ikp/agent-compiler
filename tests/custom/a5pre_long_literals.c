/* L and UL suffixes on integer literals: the type lists use the target's long. */
enum {
    E_SIZE = sizeof(1L),
    E_NEG = -0x80000000L < 0,
    E_TOP = (int)(-1UL >> (sizeof(1L) * 8 - 8))
};

int l_size_l(void) { return sizeof(1L); }
int l_size_lower_l(void) { return sizeof(1l); }
int l_size_hex_l(void) { return sizeof(0xFFFFFFFFL); }
int l_size_oct_l(void) { return sizeof(037777777777L); }
int l_size_ul(void) { return sizeof(1UL); }
int l_size_ul_mixed(void) { return sizeof(1uL) + sizeof(1Lu) * 100 + sizeof(1lU) * 10000; }
int l_size_ul_lower(void) { return sizeof(1ul); }
int l_neg_l(void) { return -1L < 0; }
int l_hex_l_pos(void) { return 0xFFFFFFFFL > 0; }
int l_neg_hex_l(void) { return -0x80000000L < 0; }
int l_shl_40(void) { return (int)((1L << 40) >> 38); }
int l_shift_ul(void) { return (int)((1UL << 31) >> 31); }
int l_top_byte(void) { return (int)(-1UL >> (sizeof(1L) * 8 - 8)); }
int l_neg_ul(void) { return -1UL > 0; }
int l_enum_size(void) { return E_SIZE; }
int l_enum_neg(void) { return E_NEG; }
int l_enum_top(void) { return E_TOP; }
int l_case(int x)
{
    switch (x) {
    case sizeof(1L) * 0 + 5L: return 1;
    case E_SIZE: return 2;
    case 9UL: return 3;
    default: return 0;
    }
}
