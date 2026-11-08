int f(int k)
{
    int m[3][4];
    int (*pa)[4];
    int *row;
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 4; j++)
            m[i][j] = i * 10 + j;
    pa = m;
    row = m[1];
    return m[2][3] * 1000000 + pa[1][2] * 10000 + (*(pa + 2))[1] * 100 + *(*(m + 1) + k) + row[3] * 0;
}
