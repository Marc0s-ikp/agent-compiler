int unused(int x);
int later(int x);

int f(int x)
{
    return later(x) + 1;
}

int later(int x)
{
    return x * 2;
}
