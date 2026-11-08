typedef int T;

T global_t = 3;

int local_var_named_t()
{
    int T = 5;
    return T + 1;
}

int param_named_t(int T)
{
    return T * 2;
}

int after_shadow()
{
    T x;
    x = 7;
    return x;
}

int inner_var_then_type()
{
    int r;
    {
        int T = 10;
        r = T;
    }
    {
        T y = 20;
        r = r + y;
    }
    return r;
}
