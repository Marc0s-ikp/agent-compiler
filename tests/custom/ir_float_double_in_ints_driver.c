double call_gcc_pairs(void);
double call_ours_pairs(void);
double ours_pairs(double d0, double d1, double d2, double d3, double d4,
                  double d5, double d6, double d7, double d8, double d9,
                  float f, int tail);

double gcc_pairs(double d0, double d1, double d2, double d3, double d4,
                 double d5, double d6, double d7, double d8, double d9,
                 float f, int tail)
{
    return d0 + d1 * 2 + d2 * 3 + d3 * 4 + d4 * 5 + d5 * 6 + d6 * 7 + d7 * 8 +
           d8 * 9 + d9 * 10 + f * 11 + tail * 12;
}

int main(void)
{
    double want = gcc_pairs(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.25, -10.75, 0.125f, 12);
    if (call_gcc_pairs() != want) return 1;
    if (call_ours_pairs() != want) return 2;
    if (ours_pairs(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.25, -10.75, 0.125f, 12) != want) return 3;
    return 0;
}
