/* Eight doubles use up fa0-fa7. The ninth and tenth double then travel in
   integer register pairs (a0/a1, a2/a3), a float in a4 as 32 bits, and the
   next int in a5. gcc_pairs is defined by the GCC driver. */
double gcc_pairs(double d0, double d1, double d2, double d3, double d4,
                 double d5, double d6, double d7, double d8, double d9,
                 float f, int tail);

double ours_pairs(double d0, double d1, double d2, double d3, double d4,
                  double d5, double d6, double d7, double d8, double d9,
                  float f, int tail)
{
    return d0 + d1 * 2 + d2 * 3 + d3 * 4 + d4 * 5 + d5 * 6 + d6 * 7 + d7 * 8 +
           d8 * 9 + d9 * 10 + f * 11 + tail * 12;
}

double call_gcc_pairs(void)
{
    return gcc_pairs(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.25, -10.75, 0.125f, 12);
}

double call_ours_pairs(void)
{
    return ours_pairs(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.25, -10.75, 0.125f, 12);
}
