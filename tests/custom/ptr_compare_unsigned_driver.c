int f(int *p, int *q);

int main()
{
    int *hi = (int *)0x80000000UL;
    int *lo = (int *)0x1000;
    /* addresses compare unsigned: hi > lo */
    if (f(hi, lo) != 2 + 8) return 1;
    if (f(lo, hi) != 1 + 4) return 2;
    return 0;
}
