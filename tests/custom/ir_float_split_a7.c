/* A double that finds only a7 free: eight doubles take fa0-fa7, seven ints
   take a0-a6, and the ninth double is split (low word a7, high word at 0(sp)
   of the caller); the int after it is the next stack word, 4(sp).
   Checked as ours -> ours (call_ours_split) and GCC caller -> our callee (the
   driver). The direction "our caller -> GCC callee" is NOT tested here: GCC's
   callee is correct, but it reads the split double with one fld from an
   address that is 4 mod 8, and spike pk emulates a misaligned fld wrongly.
   tests/abi_check covers that direction with a bytewise check. See
   docs/BLOCKERS.md "Known limits left by feature 7". */
double ours_split(double d0, double d1, double d2, double d3, double d4,
                  double d5, double d6, double d7, int a, int b, int c, int d,
                  int e, int f, int g, double x, int tail)
{
    return d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + a + b + c + d + e + f + g +
           x * 100 + tail * 1000;
}

double call_ours_split(void)
{
    return ours_split(1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 2.5, 9);
}
