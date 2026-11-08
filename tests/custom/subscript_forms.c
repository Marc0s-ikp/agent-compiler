int f(int *a, int i)
{
    int loc[4];
    loc[0] = a[i];
    loc[1] = *(a + i);
    loc[2] = i[a];
    loc[3] = 2[loc];
    return loc[0] * 1000 + loc[1] * 100 + loc[3];
}
