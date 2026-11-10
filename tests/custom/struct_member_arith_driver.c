int char_wrap(void); int uchar_wrap(void); int char_signed_load(void); int char_compound(void);
int float_arith(void); int double_arith(void); int float_to_double(void); int int_to_float(void);
int incdec(void); int compare(void); int mixed_expression(void);

int main()
{
    if (char_wrap() != -128) return 1;
    if (uchar_wrap() != 1) return 2;
    if (char_signed_load() != 994) return 3;
    if (char_compound() != -27900) return 4;
    if (float_arith() != 13) return 5;
    if (double_arith() != 5) return 6;
    if (float_to_double() != 75) return 7;
    if (int_to_float() != 35) return 8;
    if (incdec() != 728) return 9;
    if (compare() != 11) return 10;
    if (mixed_expression() != 70) return 11;
    return 0;
}
