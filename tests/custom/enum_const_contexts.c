enum { N = 4, M = N * 2, NEG = -N };

int g[N] = {1, N, M, NEG};
int big[M];

int fill()
{
    int loc[N + 1];
    int i;
    for (i = 0; i < N + 1; i++) loc[i] = i;
    for (i = 0; i < M; i++) big[i] = i * 2;
    return loc[N] * 100 + big[M - 1];
}

int get(int i)
{
    return g[i];
}

int tern(int x)
{
    return x ? N : M;
}
