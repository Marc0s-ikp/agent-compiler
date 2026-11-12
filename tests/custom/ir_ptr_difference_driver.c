int idiff(int *a, int *b);
int cdiff(char *a, char *b);
int sdiff(short *a, short *b);
int rows(int (*p)[3], int (*q)[3]);
int mixed(int *a);

int main()
{
    int x[20];
    short y[20];
    char z[20];
    int m[6][3];
    if (idiff(&x[9], &x[2]) != 7) return 1;
    if (idiff(&x[2], &x[9]) != -7) return 2;
    if (idiff(x, x) != 0) return 3;
    if (cdiff(z + 15, z + 3) != 12) return 4;
    if (cdiff(z + 3, z + 15) != -12) return 5;
    if (sdiff(y + 1, y + 18) != -17) return 6;
    if (sdiff(y + 18, y + 1) != 17) return 7;
    if (rows(&m[1], &m[5]) != 4) return 8;
    if (rows(&m[5], &m[1]) != -4) return 9;
    if (mixed(x) != 5 - 50) return 10;
    return 0;
}
