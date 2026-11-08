int f(int x)
{
    int i;
    int s = 0;
    for (i = 0; i < x; i++) {
        if (i % 2)
            continue;
        s = s + i;
    }
    return s * 100 + i;
}
