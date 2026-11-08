int use_helper(int x);

static int helper(int x)
{
    return x * 100;
}

int main()
{
    if (use_helper(5) != 7) return 1;
    if (helper(2) != 200) return 2;
    return 0;
}
