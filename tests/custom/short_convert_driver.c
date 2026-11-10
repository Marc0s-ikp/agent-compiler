extern int int_to_short(int v), int_to_ushort(int v), short_to_char(short v), short_to_uchar(short v);
extern int char_to_short(signed char c), uchar_to_short(unsigned char c), schar_to_ushort(signed char c);
extern int short_to_ushort(short v), ushort_to_short(unsigned short v);
extern unsigned int short_to_uint(short v), ushort_to_uint(unsigned short v);
extern int ushort_to_char(unsigned short v), double_cast_chain(int v), shortcast_expr(void);
extern short float_to_short(float f), double_to_short(double d);
extern unsigned short double_to_ushort(double d), float_to_ushort(float f);
extern float short_to_float(short s), ushort_to_float(unsigned short s);
extern double short_to_double(short s), ushort_to_double(unsigned short s);
extern double store_roundtrip(double d), compound_double(void), compound_double_neg(void), init_double(void);
extern float store_roundtrip_f(float f);
extern int compare_float(void);

int main(void)
{
    if (int_to_short(65535) != -1 || int_to_short(32768) != -32768 || int_to_short(-5) != -5) return 1;
    if (int_to_ushort(-1) != 65535 || int_to_ushort(65537) != 1) return 2;
    if (short_to_char(0x1234) != 0x34 || short_to_char(0x1280) != -128 || short_to_char(-1) != -1) return 3;
    if (short_to_uchar(-1) != 255 || short_to_uchar(0x1234) != 0x34) return 4;
    if (char_to_short(-1) != -1 || char_to_short(127) != 127) return 5;
    if (uchar_to_short(255) != 255 || uchar_to_short(128) != 128) return 6;
    if (schar_to_ushort(-1) != 65535 || schar_to_ushort(5) != 5) return 7;
    if (short_to_ushort(-1) != 65535 || short_to_ushort(-32768) != 32768) return 8;
    if (ushort_to_short(65535) != -1 || ushort_to_short(40000) != -25536) return 9;
    if (short_to_uint(-1) != 4294967295u || ushort_to_uint(65535) != 65535u) return 10;
    if (ushort_to_char(0xff80) != -128 || ushort_to_char(0x7f) != 127) return 11;
    if (double_cast_chain(0x1ff) != -1 || double_cast_chain(0x17f) != 127) return 12;
    if (shortcast_expr() != 4464 + 4465 + 8928) return 13;
    if (float_to_short(-3.7f) != -3 || double_to_short(-1234.9) != -1234) return 14;
    if (double_to_short(32767.0) != 32767 || double_to_short(-32768.0) != -32768) return 15;
    if (double_to_ushort(65535.0) != 65535 || float_to_ushort(40000.5f) != 40000) return 16;
    if (short_to_float(-5) != -5.0f || short_to_double(-32768) != -32768.0) return 17;
    if (ushort_to_double(65535) != 65535.0 || ushort_to_float(40000) != 40000.0f) return 18;
    if (store_roundtrip(-2.5) != -2.0 || store_roundtrip(-32768.0) != -32768.0) return 19;
    if (store_roundtrip_f(-9.99f) != -9.0) return 20;
    if (compound_double() != 12.0 || compound_double_neg() != -12.0) return 21;
    if (init_double() != -7.0) return 22;
    if (compare_float() != 3) return 23;
    return 0;
}
