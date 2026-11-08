static int hidden = 7;
static int zeroed;
static int tbl[3] = {1, 2, 3};

int get_hidden()
{
    hidden = hidden + 1;
    return hidden;
}

int get_zero()
{
    return zeroed;
}

int tbl_sum()
{
    return tbl[0] + tbl[1] + tbl[2];
}
