/* Ten floating arguments interleaved with ints and floats: the first eight
   floating values take fa0-fa7, the rest travel in integer registers and the
   stack. gcc_mixed is defined by the GCC-compiled driver, so this file is
   checked as caller (call_gcc_mixed) and as callee (ours_mixed, called by the
   driver), and against itself (call_ours_mixed). */
double gcc_mixed(int i0, double d0, float f0, int i1, double d1, double d2,
                 float f1, int i2, double d3, double d4, float f2, int i3,
                 double d5, double d6, int i4, float f3, double d7, double d8,
                 int i5, double d9);

double ours_mixed(int i0, double d0, float f0, int i1, double d1, double d2,
                  float f1, int i2, double d3, double d4, float f2, int i3,
                  double d5, double d6, int i4, float f3, double d7, double d8,
                  int i5, double d9)
{
    return i0 * 1.0 + d0 * 2 + f0 * 3 + i1 * 4.0 + d1 * 5 + d2 * 6 + f1 * 7 +
           i2 * 8.0 + d3 * 9 + d4 * 10 + f2 * 11 + i3 * 12.0 + d5 * 13 +
           d6 * 14 + i4 * 15.0 + f3 * 16 + d7 * 17 + d8 * 18 + i5 * 19.0 +
           d9 * 20;
}

double call_gcc_mixed(void)
{
    return gcc_mixed(1, 2.5, 3.5f, 4, 5.5, 6.5, 7.5f, 8, 9.5, 10.5, 11.5f, 12,
                     13.5, 14.5, 15, 16.5f, 17.5, 18.5, 19, 20.5);
}

double call_ours_mixed(void)
{
    return ours_mixed(1, 2.5, 3.5f, 4, 5.5, 6.5, 7.5f, 8, 9.5, 10.5, 11.5f, 12,
                      13.5, 14.5, 15, 16.5f, 17.5, 18.5, 19, 20.5);
}
