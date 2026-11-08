int f(int x)
{
    int y = x;
    {
        int y = x + 1;
        {
            int y = x + 2;
            x = y;
        }
        x = x + y;
    }
    return x * 100 + y;
}
