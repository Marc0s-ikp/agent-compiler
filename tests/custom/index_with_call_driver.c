int f(int *a, int i);

int main()
{
    int x[8];
    int k;
    for (k = 0; k < 8; k++) x[k] = k * 10;
    /* h(v)=v+1. i=1: a[3] = h(2)+a[1] = 3+10 = 13. a[2] = h(a[0])*2 = 2. return a[3]*100 + a[2] */
    if (f(x, 1) != 1300 + 2) return 1;
    if (x[3] != 13 || x[2] != 2) return 2;
    return 0;
}
