char *names[] = {"ab", "cde", "", "f"};

int name_len(int i)
{
    int n;
    n = 0;
    while (names[i][n]) n++;
    return n;
}

int count(void)
{
    return sizeof(names) / sizeof(names[0]);
}

int pick(int i, int j)
{
    return names[i][j];
}
