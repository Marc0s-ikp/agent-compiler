int f(int a, int b, int c, int d, int e, int g, int h, int i, int j, int k, int l, int m);

int main()
{
    /* a=1 h=7 i=8 j=9 k=10 l=11 m=12: 1 + 144 + 121 + 2900 + 72 + 56 */
    if (f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) != 3294) return 1;
    return 0;
}
