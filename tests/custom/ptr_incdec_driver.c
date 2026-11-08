int f(int *a);

int main()
{
    int x[5];
    x[0] = 1; x[1] = 2; x[2] = 4; x[3] = 5; x[4] = 6;
    /* reads: 1, 2, then x[1] incremented -> 3, x[3]=5, x[2]=4, x[1]=3, x[0]=1; final p-a = 1 */
    if (f(x) != 12354311) return 1;
    if (x[1] != 3) return 2;
    if (x[0] != 1) return 3;
    return 0;
}
