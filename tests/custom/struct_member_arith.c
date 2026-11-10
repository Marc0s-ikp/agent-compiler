/* Struct members used in float, double and char arithmetic: the member's own
   type decides conversions, truncation and sign extension. */
/* plain char is unsigned on this target; 's' is the signed one */
struct M { char c; unsigned char u; signed char s; float f; double d; int i; };

int char_wrap(void)
{
    struct M m;
    m.s = 127;
    m.s = m.s + 1;                           /* stored as -128 */
    return m.s;
}
int uchar_wrap(void)
{
    struct M m;
    m.u = 255;
    m.u = m.u + 2;                           /* 1 */
    return m.u;
}
int char_signed_load(void)
{
    struct M m;
    m.s = -3;
    m.c = 253;                               /* plain char: 253, not -3 */
    return m.s * 2 + (m.c == 253) * 1000;    /* -6 + 1000 */
}
int char_compound(void)
{
    struct M m;
    m.s = 100;
    m.s += 100;                              /* -56 */
    m.s >>= 1;                               /* -28 */
    m.c = 100;
    m.c += 100;                              /* 200, unsigned */
    m.c >>= 1;                               /* 100 */
    return m.s * 1000 + m.c;                 /* -28000 + 100 */
}
int float_arith(void)
{
    struct M m;
    m.f = 1.5f;
    m.f = m.f * 4.0f + 0.5f;                 /* 6.5 */
    return (int)(m.f * 2.0f);                /* 13 */
}
int double_arith(void)
{
    struct M m;
    m.d = 0.1;
    m.d += 0.4;
    m.d *= 10.0;
    return (int)m.d;                         /* 5 */
}
int float_to_double(void)
{
    struct M m;
    m.f = 0.5f;
    m.d = m.f + 0.25;                        /* 0.75 */
    return (int)(m.d * 100.0);
}
int int_to_float(void)
{
    struct M m;
    m.i = 7;
    m.f = m.i;
    m.f /= 2;                                /* 3.5 */
    return (int)(m.f * 10.0f);               /* 35 */
}
int incdec(void)
{
    struct M m;
    m.i = 5; m.f = 1.0f; m.c = 9;
    m.i++; ++m.i; m.f++; m.c--;
    return m.i * 100 + (int)m.f * 10 + m.c;  /* 700+20+8 */
}
int compare(void)
{
    struct M m;
    m.f = 2.5f; m.d = 2.5;
    return (m.f == m.d) + (m.f < 3) * 10 + (m.d > 3.0) * 100;   /* 11 */
}
int mixed_expression(void)
{
    struct M m;
    m.c = 3; m.i = 4; m.f = 0.5f; m.d = 2.0;
    return (int)((m.c + m.i) * m.f * m.d * 10.0);   /* 70 */
}
