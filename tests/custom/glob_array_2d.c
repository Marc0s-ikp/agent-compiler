int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
int z[3][2];

int at(int i, int j)
{
    return m[i][j];
}

int fill()
{
    int i;
    int j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 2; j++)
            z[i][j] = i * 10 + j;
    return z[2][1] + z[1][0] + z[0][0];
}
