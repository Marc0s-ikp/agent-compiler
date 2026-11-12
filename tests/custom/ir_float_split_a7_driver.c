double call_ours_split(void);
double ours_split(double d0, double d1, double d2, double d3, double d4,
                  double d5, double d6, double d7, int a, int b, int c, int d,
                  int e, int f, int g, double x, int tail);

int main(void)
{
    double want = 36 + 28 + 250 + 9000;  /* 9314 */
    if (call_ours_split() != want) return 2;                                        /* ours -> ours */
    if (ours_split(1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 2.5, 9) != want) return 3; /* GCC -> ours */
    return 0;
}
