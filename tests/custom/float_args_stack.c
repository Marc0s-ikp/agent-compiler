/* Arguments that overflow both register files. The unused fa/a registers
   must not be filled out of order. */

/* 6 ints in a0-a5, 8 doubles in fa0-fa7. The 9th double takes the aligned
   pair a6:a7; the next int is on the stack (sp+0) and the double after it
   is 8-aligned at sp+8. (A double split across a7 and the stack is what GCC
   emits after 7 ints, but GCC's own callee reads it with a misaligned fld
   that gives a wrong value under spike pk, so it is not tested here.) */
double stk(int a, int b, int c, int d, int e, int f,
           double d1, double d2, double d3, double d4,
           double d5, double d6, double d7, double d8,
           double x, int y, double z)
{
    return x * 100.0 + y * 10.0 + z + a + f;
}

/* 8 ints, 8 floats, then two floats on the stack at sp+0 and sp+4 */
float stkf(int a, int b, int c, int d, int e, int f, int g, int h,
           float f1, float f2, float f3, float f4,
           float f5, float f6, float f7, float f8,
           float x, float z)
{
    return x * 10.0f + z + h;
}

/* float at sp+0, double 8-aligned at sp+8 */
double stkd2(int a, int b, int c, int d, int e, int f, int g, int h,
             double d1, double d2, double d3, double d4,
             double d5, double d6, double d7, double d8,
             float x, double y)
{
    return y * 10.0 + x;
}

/* ints on the stack after the registers fill, doubles in registers */
double stki(int a, int b, int c, int d, int e, int f, int g, int h,
            int i, int j, double k)
{
    return k * 1000.0 + i * 10 + j;
}

/* a double on the stack followed by a char and an int on the stack */
int stkmix(int a, int b, int c, int d, int e, int f, int g, int h,
           double x, char c1, int n)
{
    return c1 + n + h;
}
