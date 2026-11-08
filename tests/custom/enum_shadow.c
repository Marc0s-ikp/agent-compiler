enum { RED, GREEN, BLUE };

int local_hides(int x)
{
    int r = BLUE;
    {
        int BLUE = x;
        r = r + BLUE;
    }
    return r * 10 + BLUE;
}

int param_hides(int GREEN)
{
    return GREEN;
}

int block_enum_one()
{
    enum { K = 1 };
    return K;
}

int block_enum_two()
{
    enum { K = 2, L };
    return K + L;
}

int block_enum_hides()
{
    int r;
    {
        enum { GREEN = 40 };
        r = GREEN;
    }
    return r + GREEN;
}

int outer_enum_after()
{
    return RED + GREEN * 10 + BLUE * 100;
}
