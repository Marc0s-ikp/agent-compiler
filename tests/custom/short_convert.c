/* Casts and conversions between char, short, int, float and double. */
int int_to_short(int v) { return (short)v; }
int int_to_ushort(int v) { return (unsigned short)v; }
int short_to_char(short v) { return (signed char)v; }
int short_to_uchar(short v) { return (unsigned char)v; }
int char_to_short(signed char c) { return (short)c; }
int uchar_to_short(unsigned char c) { return (short)c; }
int schar_to_ushort(signed char c) { return (unsigned short)c; }
int short_to_ushort(short v) { return (unsigned short)v; }
int ushort_to_short(unsigned short v) { return (short)v; }
unsigned int short_to_uint(short v) { return (unsigned int)v; }
unsigned int ushort_to_uint(unsigned short v) { return (unsigned int)v; }
int ushort_to_char(unsigned short v) { return (signed char)v; }
int double_cast_chain(int v) { return (short)(signed char)(short)v; }

int shortcast_expr(void) { int a = 70000; return (short)a + (short)(a + 1) + (unsigned short)(a * 2); }

/* floating point */
short float_to_short(float f) { return (short)f; }
short double_to_short(double d) { return (short)d; }
unsigned short double_to_ushort(double d) { return (unsigned short)d; }
unsigned short float_to_ushort(float f) { return (unsigned short)f; }
float short_to_float(short s) { return (float)s; }
double short_to_double(short s) { return (double)s; }
double ushort_to_double(unsigned short s) { return (double)s; }
float ushort_to_float(unsigned short s) { return (float)s; }
double store_roundtrip(double d) { short s; s = d; return s; }
float store_roundtrip_f(float f) { short s; s = f; return s; }
double compound_double(void) { short s = 10; s += 2.9; return s; }       /* 12.9 -> 12 */
double compound_double_neg(void) { short s = -10; s += -2.9; return s; }   /* -12.9 -> -12 */
double init_double(void) { short s = -7.9; return s; }
int compare_float(void) { short s = -2; return (s < -1.5) + 2 * (s == -2.0f); }
