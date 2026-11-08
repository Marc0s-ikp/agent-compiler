int t[5] = {1, 2, 3, 4, 5};
int u[6] = {7, 8};
int v[] = {10, 20, 30};
int w[3] = {-1, 0x10, 1 + 1};

int sum_t()
{
    int i;
    int s = 0;
    for (i = 0; i < 5; i++) s = s + t[i];
    return s;
}

int tail_u()
{
    return u[0] * 10000 + u[1] * 1000 + u[2] + u[3] + u[4] + u[5];
}

int last_v()
{
    return v[2];
}

int get_w(int i)
{
    return w[i];
}
