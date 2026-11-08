int v = 3;

int local_typedef_hides_variable()
{
    typedef int v;
    v x;
    x = 4;
    return x;
}

int variable_visible_again()
{
    return v;
}

int typedef_ends_with_block()
{
    int r;
    {
        typedef int U;
        U a = 1;
        r = a;
    }
    {
        int U = 5;
        r = r + U;
    }
    return r;
}

int same_name_two_functions_a()
{
    typedef int W;
    W w = 1;
    return w;
}

int same_name_two_functions_b()
{
    typedef int *W;
    int n = 2;
    W w = &n;
    return *w;
}
