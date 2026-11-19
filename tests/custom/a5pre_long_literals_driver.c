/* Every expected value is the same expression compiled by GCC for this target, so sizeof(long)
   and the literal types follow the target. None of the literals is a DECIMAL one above LONG_MAX:
   there GCC's default C17 gives long long where the compiler follows C90 (unsigned long), and
   a4_literal_types covers that rule. 1L << 40 is defined only where long is 8 bytes, so the
   driver skips it otherwise (the guard is here, never in the compiler). */
int l_size_l(void), l_size_lower_l(void), l_size_hex_l(void), l_size_oct_l(void), l_size_ul(void);
int l_size_ul_mixed(void), l_size_ul_lower(void), l_neg_l(void), l_hex_l_pos(void), l_neg_hex_l(void);
int l_shl_40(void), l_shift_ul(void), l_top_byte(void), l_neg_ul(void);
int l_enum_size(void), l_enum_neg(void), l_enum_top(void), l_case(int x);

int main(void)
{
    if (l_size_l() != sizeof(1L)) return 1;
    if (l_size_lower_l() != sizeof(1l)) return 2;
    if (l_size_hex_l() != sizeof(0xFFFFFFFFL)) return 3;
    if (l_size_oct_l() != sizeof(037777777777L)) return 4;
    if (l_size_ul() != sizeof(1UL)) return 5;
    if (l_size_ul_mixed() != (int)(sizeof(1uL) + sizeof(1Lu) * 100 + sizeof(1lU) * 10000)) return 6;
    if (l_size_ul_lower() != sizeof(1ul)) return 7;
    if (l_neg_l() != (-1L < 0)) return 8;
    if (l_hex_l_pos() != (0xFFFFFFFFL > 0)) return 9;
    if (l_neg_hex_l() != (-0x80000000L < 0)) return 10;
    if (sizeof(long) >= 8 && l_shl_40() != (int)((1L << 40) >> 38)) return 11;
    if (l_shift_ul() != (int)((1UL << 31) >> 31)) return 12;
    if (l_top_byte() != (int)(-1UL >> (sizeof(long) * 8 - 8))) return 13;
    if (l_neg_ul() != (-1UL > 0)) return 14;
    if (l_enum_size() != sizeof(1L)) return 15;
    if (l_enum_neg() != (-0x80000000L < 0)) return 16;
    if (l_enum_top() != (int)(-1UL >> (sizeof(long) * 8 - 8))) return 17;
    if (l_case(5) != 1 || l_case((int)sizeof(1L)) != ((int)sizeof(1L) == 5 ? 1 : 2)) return 18;
    if (l_case(9) != 3 || l_case(7) != 0) return 19;
    return 0;
}
