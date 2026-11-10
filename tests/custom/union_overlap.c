/* All members start at offset 0: writing one and reading another sees the
   same bytes (little-endian RV32). */
union W { int i; char c[4]; float f; };
union L { double d; int w[2]; };

int low_byte(void) { union W u; u.i = 0x12345678; return u.c[0]; }       /* 0x78 */
int high_byte(void) { union W u; u.i = 0x12345678; return u.c[3]; }      /* 0x12 */
int write_bytes(void)
{
    union W u;
    u.c[0] = 1; u.c[1] = 0; u.c[2] = 0; u.c[3] = 0;
    return u.i;                                                          /* 1 */
}
int float_bits(void) { union W u; u.f = 1.0f; return u.i == 0x3f800000; }
int double_words(void)
{
    union L u;
    u.d = 1.0;                       /* 0x3ff00000 00000000 */
    return u.w[1] == 0x3ff00000 && u.w[0] == 0;
}
int last_write_wins(void)
{
    union W u;
    u.i = 5;
    u.f = 2.0f;
    return u.i == 0x40000000;
}
int addresses_equal(void)
{
    union W u;
    return (char *)&u.i == (char *)&u.f && (char *)&u.i == &u.c[0];
}
