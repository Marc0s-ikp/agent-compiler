int g = 5;

int local_hides()
{
    int g = 1;
    {
        int g = 2;
        g = g + 1;
    }
    return g;
}

int param_hides(int g)
{
    return g * 2;
}

int global_after()
{
    return g;
}

int set_global(int v)
{
    g = v;
    return g;
}
