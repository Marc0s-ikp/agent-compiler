static int helper(int x)
{
    return x + 1;
}

int use_helper(int x)
{
    return helper(helper(x));
}
